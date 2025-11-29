# input 和 output 内建函数使用文档

## 概述

本编译器支持两个特殊的内建函数 `input()` 和 `output()`，用于与 FPGA 硬件的键盘和 LCD 设备进行交互。

## 指令规格

这两个内建函数对应于 MIPS ISA 中的两条 I 型指令：

### 1. input (opcode=01110)
- **汇编格式**: `input rd`
- **功能**: 从键盘读取 32 位数据到寄存器 rd
- **硬件行为**: 
  - `rd = keyboard_in`
  - `keyboard_ack = 1` (握手信号)

### 2. output (opcode=01111)
- **汇编格式**: `output rd`
- **功能**: 将寄存器 rd 的低 8 位输出到 LCD
- **硬件行为**:
  - `LCD = rd[7:0]`
  - `lcd_write = 1` (写信号)

## C 语言使用方法

### input() 函数

```c
int input();
```

- **参数**: 无
- **返回值**: int (32 位整数)
- **说明**: 从键盘读取一个整数

**示例**:
```c
int x = input();        // 从键盘读取整数
int y = input();        // 可以多次调用
int sum = x + y;
```

### output() 函数

```c
void output(int value);
```

- **参数**: int value - 要输出的整数（只使用低 8 位）
- **返回值**: 无
- **说明**: 将整数的低 8 位输出到 LCD

**示例**:
```c
int x = 42;
output(x);              // 输出 42 到 LCD

int result = x + 10;
output(result);         // 输出计算结果
```

## 完整示例

### 示例 1: 简单的输入输出
```c
int main() {
    int x = input();     // 从键盘读取
    output(x);           // 输出到 LCD
    return 0;
}
```

### 示例 2: 计算器
```c
int main() {
    int a = input();     // 读取第一个数
    int b = input();     // 读取第二个数
    int sum = a + b;     // 计算和
    output(sum);         // 输出结果
    return 0;
}
```

### 示例 3: 输出字符串（逐字符）
```c
int main() {
    int c = 72;          // 'H' 的 ASCII 码
    output(c);
    c = 69;              // 'E'
    output(c);
    c = 76;              // 'L'
    output(c);
    output(c);           // 再输出一次 'L'
    c = 79;              // 'O'
    output(c);
    return 0;
}
```

## 编译器实现细节

### 编译流程

1. **词法分析 (Lexer)**: `input` 和 `output` 作为普通标识符处理
2. **语法分析 (Parser)**: 作为函数调用解析
3. **语义分析 (Semantic Analyzer)**: 
   - `input()` 被识别为返回 `int` 的内建函数，不接受参数
   - `output(int)` 被识别为返回 `void` 的内建函数，接受一个 `int` 参数
4. **IR 生成 (IRBuilder)**: 
   - `input()` → `{"input", tmp, "", ""}`
   - `output(x)` → `{"output", x, "", ""}`
5. **代码生成 (CodeGen)**: 
   - `input` IR → `input $rd` 汇编指令
   - `output` IR → `output $rs` 汇编指令

### 生成的汇编示例

C 代码:
```c
int x = input();
output(x);
```

生成的汇编:
```asm
input $t0           # 从键盘读取到 $t0
sw $t0, -1($fp)    # 保存到栈上的局部变量
lw $t1, -1($fp)    # 加载局部变量
output $t1          # 输出到 LCD
```

## 注意事项

1. **不是标准 C 库函数**: `input()` 和 `output()` 不是 `scanf()` 或 `printf()`，它们是编译器特殊处理的内建函数。

2. **硬件限制**: 
   - `output()` 只输出低 8 位（0-255），适合输出 ASCII 字符
   - 对于大于 255 的数字，只有最低字节会被显示

3. **不支持格式化**: 无法像 `printf()` 那样格式化输出，只能输出单个字节。

4. **声明不是必需的**: 编译器内置了这两个函数，不需要声明即可使用。

## 错误处理

编译器会检查以下错误：

- `input()` 使用了参数：`input() takes no arguments`
- `output()` 没有参数或参数数量错误：`output() takes exactly one argument`
- `output()` 的参数类型不是 int：`output() argument must be int`

## 测试

测试文件位于 `tests/programs/` 目录：
- `test_io_simple.c` - 基本输入输出测试
- `test_io.c` - 输入输出和运算
- `test_io_calc.c` - 双输入加法计算器

编译测试：
```bash
./main ../tests/programs/test_io_simple.c -o ../tests/programs/test_io_simple.s
```
