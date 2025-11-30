# Assembler 开发文档

## 概述

本文档记录了 MIPS-compiler 项目中汇编器模块的设计与实现细节。汇编器负责将汇编语言源代码转换为机器码，支持自定义 RISC 指令集架构 (ISA)。

## 模块架构

汇编器采用分层架构设计：

```
┌─────────────────────────────────────────────────────────────┐
│                    assembler_main.cpp                       │
│                  (命令行入口 + 文件 I/O)                      │
├─────────────────────────────────────────────────────────────┤
│                      Assembler 类                            │
│                   (assembler.hpp/cpp)                        │
│              ┌─────────────────────────────┐                │
│              │        汇编流水线           │                 │
│   源代码 → [AsmLexer] → [AsmParser] → [AsmEncoder] → 机器码  │
│              └─────────────────────────────┘                │
├─────────────────────────────────────────────────────────────┤
│                      FileIO 工具类                           │
│                (HEX/MIF 文件读写)                            │
└─────────────────────────────────────────────────────────────┘
```

### 顶层接口

#### Assembler 类

**文件**: `include/assembler/assembler.hpp`, `src/assembler/assembler.cpp`

**功能**: 汇编器顶层类，封装完整汇编流水线

**流水线阶段接口**:
- `runLexer()`: 执行词法分析阶段
- `runParser()`: 执行语法分析阶段
- `runEncoder()`: 执行编码生成阶段
- `assemble()`: 执行完整汇编流水线

**输出结构 AssembleResult**:
```cpp
struct AssembleResult {
    bool success;                       // 是否成功
    std::vector<uint32_t> imem;         // 指令内存（代码段）
    std::vector<uint32_t> dmem;         // 数据内存（数据段）
    std::vector<std::string> errors;    // 错误列表
    std::vector<std::string> warnings;  // 警告列表
};
```

#### assembler_main.cpp

**文件**: `src/main/assembler_main.cpp`

**功能**: 汇编器独立命令行入口

**使用方法**:
```bash
./assembler <input.s> -o <output_base> [-f format]
```

**参数**:
- `input.s`: 汇编源文件
- `-o output`: 输出文件名（不含扩展名）
- `-f format`: 输出格式 (hex, mif, both，默认 both)

**示例**:
```bash
./assembler test.s -o test           # 输出 test.hex, test.mif
./assembler test.s -o test -f hex    # 只输出 test.hex
```

---

### 流水线组件

### 1. AsmLexer (词法分析器)

**文件**: `include/assembler/asm_lexer.hpp`, `src/assembler/asm_lexer.cpp`

**功能**: 将汇编源代码文本分解为 Token 序列

**支持的 Token 类型**:
- `DIRECTIVE`: 伪指令 (`.text`, `.data`, `.word`, `.space`)
- `LABEL`: 标签定义 (`loop:`, `main:`)
- `INSTRUCTION`: 指令助记符 (`add`, `sub`, `lw`, `beq`, etc.)
- `REGISTER`: 寄存器 (`$r0` - `$r31`，支持 MIPS 风格别名)
- `NUMBER`: 数字立即数 (十进制、十六进制)
- `IDENTIFIER`: 标识符/标签引用
- `COMMA`, `LPAREN`, `RPAREN`, `NEWLINE`, `COMMENT`

**特性**:
- 行号跟踪，便于错误报告
- 支持注释 (`#` 开头)
- 大小写不敏感的指令识别

### 2. AsmParser (语法分析器)

**文件**: `include/assembler/asm_parser.hpp`, `src/assembler/asm_parser.cpp`

**功能**: 将 Token 序列解析为结构化的指令表示

**两遍扫描策略**:
1. **第一遍**: 收集所有标签定义，构建符号表，处理 `.data`/`.word` 伪指令
2. **第二遍**: 解析指令，解析标签引用

**输出结构**:
- `ParsedInstruction`: 包含操作码、操作数、地址信息
- `SymbolTable`: 标签名到地址的映射
- `DataWord`: 数据段字（地址 + 值）
- `ParseResult`: 完整解析结果，含 `instructions`、`dataWords`、错误信息

**支持的伪指令**:
```asm
.text           # 切换到代码段
.data           # 切换到数据段
.word 0x47      # 定义一个 32 位字
.word 'G'       # 定义一个字符（ASCII 值）
```

**支持的指令格式**:
```asm
# R-type: op rd, rs, rt
add $r1, $r2, $r3

# I-type 算术: op rd, rs, imm
addi $r1, $r2, 100

# I-type 内存: op rd, offset(rs)
lw $r1, 100($r2)
sw $r1, -50($r2)

# I-type 分支: op rd, rs, label/offset
beq $r1, $r2, loop
bgt $r3, $r4, -5

# I-type 跳转寄存器: jr rs
jr $r31

# I-type I/O: input/output rd
input $r1
output $r2

# J-type: op target
j main
jal function
```

### 3. AsmEncoder (编码器)

**文件**: `include/assembler/asm_encoder.hpp`, `src/assembler/asm_encoder.cpp`

**功能**: 将解析后的指令编码为 32 位机器码

**编码函数**:
- `encodeRType()`: R-type 指令编码
- `encodeIType()`: I-type 指令编码
- `encodeJType()`: J-type 指令编码
- `encode()`: 完整程序编码

**输出格式转换**:
- `toHexString()`: 十六进制字符串
- `toBinaryString()`: 二进制字符串（带空格分隔）
- `disassembleFields()`: 字段分解显示

### 4. FileIO (文件 I/O 工具)

**文件**: `include/utils/file_io.hpp`, `src/utils/file_io.cpp`

**功能**: 统一的文件读写接口

**主要函数**:
- `readFile()`: 读取文本文件
- `writeFile()`: 写入文本文件
- `writeHex()`: 写入 Intel HEX 格式文件（支持 word 地址模式）
- `writeMif()`: 写入 MIF 格式文件
- `getBaseName()`: 获取文件基础名

---

## ISA 规范

### 指令格式

#### R-type (寄存器型)
```
31    27 26   22 21   17 16   12 11        0
+-------+------+------+------+-------------+
| opcode|  rd  |  rs  |  rt  |   zeros     |
+-------+------+------+------+-------------+
   5bit   5bit   5bit   5bit     12bit
```

**指令列表**:
| 指令 | Opcode | 操作 |
|------|--------|------|
| ADD  | 00000  | rd = rs + rt |
| SUB  | 00001  | rd = rs - rt |
| AND  | 00010  | rd = rs & rt |
| OR   | 00011  | rd = rs \| rt |
| SLL  | 00100  | rd = rs << rt |
| SRL  | 00101  | rd = rs >> rt |

#### I-type (立即数型)
```
31    27 26   22 21   17 16                0
+-------+------+------+--------------------+
| opcode|  rd  |  rs  |    immediate       |
+-------+------+------+--------------------+
   5bit   5bit   5bit       17bit (signed)
```

**指令列表**:
| 指令   | Opcode | 操作 |
|--------|--------|------|
| ADDI   | 00110  | rd = rs + imm |
| LW     | 00111  | rd = Mem[rs + imm] |
| SW     | 01000  | Mem[rs + imm] = rd |
| BEQ    | 01001  | if (rd == rs) PC += imm |
| BGT    | 01010  | if (rd > rs) PC += imm |
| JR     | 01011  | PC = rs |
| INPUT  | 01110  | rd = 输入 |
| OUTPUT | 01111  | 输出 rd |

#### J-type (跳转型)
```
31    27 26                               0
+-------+----------------------------------+
| opcode|           target                 |
+-------+----------------------------------+
   5bit              27bit
```

**指令列表**:
| 指令 | Opcode | 操作 |
|------|--------|------|
| J    | 01100  | PC = target |
| JAL  | 01101  | $r31 = PC+1; PC = target |

### 寄存器

- 32 个通用寄存器: `$r0` - `$r31`
- `$r0`: 硬连线为 0 (zero register)
- `$r31` / `$ra`: 链接寄存器 (用于 JAL 返回地址)

**MIPS 风格别名**:
| 别名 | 寄存器 | 用途 |
|------|--------|------|
| `$zero` | `$r0` | 常量 0 |
| `$v0` | `$r2` | 返回值 |
| `$t0`-`$t7` | `$r8`-`$r15` | 临时寄存器 |
| `$s0`-`$s7` | `$r16`-`$r23` | 保存寄存器 |
| `$sp` | `$r29` | 栈指针 |
| `$fp` | `$r30` | 帧指针 |
| `$ra` | `$r31` | 返回地址 |

### 内存模型

- **字寻址** (Word-addressed)
- 每个字 32 位
- 地址单位为字（不是字节）
- **分离的指令/数据内存**: imem (代码段) 和 dmem (数据段)

---

## 输出格式

### Intel HEX 格式

Intel HEX 是一种 ASCII 文本格式，用于传输二进制数据到 EPROM、微控制器等设备。

**记录格式**:
```
:LLAAAATT[DD...]CC
```

| 字段 | 说明 |
|------|------|
| `:` | 起始标记 |
| `LL` | 数据字节数 |
| `AAAA` | 16位地址（word 地址模式：每个地址对应一个 32 位字）|
| `TT` | 记录类型 (00=数据, 01=EOF, 04=扩展地址) |
| `DD` | 数据字节（大端序，每条记录 4 字节） |
| `CC` | 校验和 |

**地址模式**:
- 使用 **word 地址**（地址按字计，每个地址增 1 对应一个 32 位字）
- 输出固定 4096 条数据记录 + 1 条 EOF 记录

**校验和计算**:
```
校验和 = 二补数(所有字节之和的低8位)

例: :04000000314080000B
字节: 04 00 00 00 31 40 80 00
和: 0x04 + 0x00 + 0x00 + 0x00 + 0x31 + 0x40 + 0x80 + 0x00 = 0xF5
校验和: (~0xF5 + 1) & 0xFF = 0x0B
```

**输出文件**:
- `<name>.hex`: 指令内存 (imem)
- `<name>_dmem.hex`: 数据内存 (dmem，仅当有数据段时生成)

### MIF 格式

Memory Initialization File，Altera/Intel FPGA 使用的内存初始化格式。

**格式示例**:
```
WIDTH=32;
DEPTH=4096;

ADDRESS_RADIX=HEX;
DATA_RADIX=HEX;

CONTENT BEGIN
    0000 : 31408000;
    0001 : 314BFFFE;
    ...
END;
```

**输出文件**:
- `<name>.mif`: 指令内存 (imem)
- `<name>_dmem.mif`: 数据内存 (dmem，仅当有数据段时生成)

---

## 使用示例

### 命令行使用

```bash
# 编译项目
cd build
cmake ..
make

# 汇编单个文件
./assembler test.s -o test

# 只输出 HEX 格式
./assembler test.s -o test -f hex

# 只输出 MIF 格式
./assembler test.s -o test -f mif
```

### 程序化使用

```cpp
#include "assembler/assembler.hpp"
#include "utils/file_io.hpp"

// 读取源文件
auto sourceOpt = utils::FileIO::readFile("test.s");
std::string asmText = *sourceOpt;

// 汇编
assembler::Assembler assembler;
assembler::AssembleResult result = assembler.assemble(asmText);

if (result.success) {
    // 输出 HEX 文件
    utils::FileIO::writeHex("test.hex", result.imem, 0, true);
    
    // 如果有数据段
    if (!result.dmem.empty()) {
        utils::FileIO::writeHex("test_dmem.hex", result.dmem, 0, true);
    }
}
```

---

## 测试

### 单元测试文件

| 文件 | 测试内容 |
|------|----------|
| `tests/asm_tests/asm_lexer_tests.cpp` | 词法分析器测试 |
| `tests/asm_tests/asm_parser_tests.cpp` | 语法分析器测试 |
| `tests/asm_tests/asm_encoder_tests.cpp` | 编码器测试 |

### 运行测试

```bash
cd build
cmake ..
make
./tests_asm_tests_asm_encoder_tests
```

### 测试覆盖

- R-type 所有指令编码
- I-type 所有指令编码（含负立即数）
- J-type 所有指令编码
- 标签解析与分支偏移计算
- 立即数范围验证
- 位域精确性验证
- Intel HEX 格式输出
- 错误处理

---

## 待实现功能

- [ ] `.space` 伪指令支持
- [ ] 宏支持
- [ ] 包含文件支持
- [ ] 更详细的错误信息

---

## 文件结构

```
include/assembler/
├── assembler.hpp       # Assembler 顶层类接口
├── asm_lexer.hpp       # 词法分析器
├── asm_parser.hpp      # 语法分析器
├── asm_encoder.hpp     # 编码器
├── asm_token.hpp       # Token 定义
└── hex_writer.hpp      # HEX 写入器（已弃用，使用 FileIO）

src/assembler/
├── assembler.cpp       # Assembler 类实现
├── asm_lexer.cpp
├── asm_parser.cpp
├── asm_encoder.cpp
└── asm_token.cpp

src/main/
└── assembler_main.cpp  # 汇编器命令行入口

tests/asm_tests/
├── asm_lexer_tests.cpp
├── asm_parser_tests.cpp
└── asm_encoder_tests.cpp
```

---

## 版本历史

| 日期 | 版本 | 变更 |
|------|------|------|
| 2025-11-28 | 0.1.0 | 初始实现: AsmLexer, AsmParser, AsmEncoder |
| 2025-11-28 | 0.2.0 | 添加 Intel HEX 格式输出支持 |
| 2025-11-29 | 0.3.0 | 添加 MIF 格式输出、FileIO 工具类 |
| 2025-11-29 | 0.4.0 | 添加 `.data`/`.word` 伪指令支持，分离 imem/dmem |
| 2025-11-29 | 0.5.0 | 重构 Assembler 类，添加独立命令行入口 |
