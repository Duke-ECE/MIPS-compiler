# Assembler 开发文档

## 概述

本文档记录了 MIPS-compiler 项目中汇编器模块的设计与实现细节。汇编器负责将汇编语言源代码转换为机器码，支持自定义 RISC 指令集架构 (ISA)。

## 模块架构

汇编器由三个核心组件组成，形成完整的汇编流水线：

```
源代码 → [AsmLexer] → Token流 → [AsmParser] → 解析结果 → [AsmEncoder] → 机器码
```

### 1. AsmLexer (词法分析器)

**文件**: `include/assembler/asm_lexer.hpp`, `src/assembler/asm_lexer.cpp`

**功能**: 将汇编源代码文本分解为 Token 序列

**支持的 Token 类型**:
- `DIRECTIVE`: 伪指令 (`.text`, `.data`, `.word`, `.space`)
- `LABEL`: 标签定义 (`loop:`, `main:`)
- `INSTRUCTION`: 指令助记符 (`add`, `sub`, `lw`, `beq`, etc.)
- `REGISTER`: 寄存器 (`$r0` - `$r31`)
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
1. **第一遍**: 收集所有标签定义，构建符号表
2. **第二遍**: 解析指令，解析标签引用

**输出结构**:
- `ParsedInstruction`: 包含操作码、操作数、地址信息
- `SymbolTable`: 标签名到地址的映射
- `ParseResult`: 完整解析结果，含错误信息

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

**文件输出**:
- `writeHex()`: Intel HEX 格式文件
- `generateHexString()`: Intel HEX 格式字符串

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
- `$r31`: 链接寄存器 (用于 JAL 返回地址)

### 内存模型

- 字寻址 (Word-addressed)
- 每个字 32 位
- 地址单位为字（不是字节）

---

## Intel HEX 输出格式

### 格式说明

Intel HEX 是一种 ASCII 文本格式，用于传输二进制数据到 EPROM、微控制器等设备。

**记录格式**:
```
:LLAAAATT[DD...]CC
```

| 字段 | 说明 |
|------|------|
| `:` | 起始标记 |
| `LL` | 数据字节数 |
| `AAAA` | 16位地址 |
| `TT` | 记录类型 (00=数据, 01=EOF, 04=扩展地址) |
| `DD` | 数据字节 |
| `CC` | 校验和 |

### 校验和计算

校验和 = 二补数(所有字节之和的低8位)

```
例: :04000000314080000B
字节: 04 00 00 00 31 40 80 00
和: 0x04 + 0x00 + 0x00 + 0x00 + 0x31 + 0x40 + 0x80 + 0x00 = 0xF5
校验和: (~0xF5 + 1) & 0xFF = 0x0B
```

### 使用示例

```cpp
#include "assembler/asm_encoder.hpp"

// 写入文件
std::vector<uint32_t> code = {0x31408000, 0x00000000};
AsmEncoder::writeHex(code, "output.hex");

// 生成字符串
std::string hexStr = AsmEncoder::generateHexString(code);
```

### 输出示例

```
:0400000000000000FC
:04000400314080000B
:00000001FF
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

- [ ] MIF 格式输出 (Memory Initialization File)
- [ ] 主汇编器可执行程序
- [ ] 数据段支持 (`.data`, `.word`, `.space`)
- [ ] 宏支持
- [ ] 包含文件支持
- [ ] 更详细的错误信息

---

## 版本历史

| 日期 | 版本 | 变更 |
|------|------|------|
| 2025-11-28 | 0.1.0 | 初始实现: AsmLexer, AsmParser, AsmEncoder |
| 2025-11-28 | 0.2.0 | 添加 Intel HEX 格式输出支持 |
