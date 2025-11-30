# input 和 output 内建函数实现总结

## 实现概述

成功在编译器中实现了 `input()` 和 `output()` 两个内建函数，用于支持 MIPS ISA 中的特殊 I/O 指令。

## 修改的文件

### 1. semantic_analyzer.hpp & semantic_analyzer.cpp
- 添加了构造函数和 `initBuiltinFunctions()` 方法
- 在 `visitProgram()` 中注册内建函数到全局符号表
- 在 `visitCall()` 中添加对 `input` 和 `output` 的特殊处理和参数验证

**关键修改**:
```cpp
// 在全局作用域注册内建函数
symbols.declare("input", "int");   // int input()
symbols.declare("output", "void"); // void output(int)

// 特殊处理内建函数调用
if (node->callee == "input") {
    if (!node->arguments.empty()) {
        throw SemanticError("input() takes no arguments");
    }
    return "int";
}

if (node->callee == "output") {
    if (node->arguments.size() != 1) {
        throw SemanticError("output() takes exactly one argument");
    }
    // 验证参数类型为 int
    ...
    return "void";
}
```

### 2. ir_builder.cpp
- 在 `genCall()` 中添加对 `input` 和 `output` 的特殊处理
- 生成特殊的 IR 指令而不是普通的函数调用

**关键修改**:
```cpp
// input() → {"input", tmp, "", ""}
if (node->callee == "input") {
    std::string tmp = newTemp();
    program.emit({ "input", tmp, "", "" });
    return tmp;
}

// output(x) → {"output", x, "", ""}
if (node->callee == "output") {
    if (!node->arguments.empty()) {
        std::string arg = genExpr(node->arguments[0].get());
        program.emit({ "output", arg, "", "" });
    }
    return "";
}
```

### 3. codegen.cpp
- 修复了 `output` 指令的寄存器分配（从 `I.src1` 改为 `I.dst`）
- 确保 IR 到汇编的正确翻译

**关键修改**:
```cpp
if (op=="input") {
    std::string rd = allocateRegister(I.dst);
    out.push_back("input " + rd);
    return;
}

if (op=="output") {
    std::string rs = allocateRegister(I.dst);  // 注意：使用 I.dst 不是 I.src1
    out.push_back("output " + rs);
    return;
}
```

### 4. 文档和测试
- 创建了 `docs/builtin_io_functions.md` 使用文档
- 添加了多个测试用例：
  - `test_io_simple.c` - 基本输入输出
  - `test_io.c` - 输入输出和运算
  - `test_io_calc.c` - 双输入加法器
  - `test_io_fibonacci.c` - Fibonacci 递归计算

## 生成的汇编示例

### 简单示例
C 代码:
```c
int main() {
    int x = input();
    output(x);
    return 0;
}
```

生成的汇编:
```asm
main:
# Function prologue
addi $sp, $sp, -28
sw $fp, 1($sp)
addi $fp, $sp, 0
sw $ra, 2($sp)
input $t0           # 从键盘读取
sw $t0, -1($fp)     # 保存到局部变量
lw $t1, -1($fp)     # 加载局部变量
output $t1          # 输出到 LCD
addi $t2, $zero, 0
addi $v0, $t2, 0
# Function epilogue
lw $ra, 2($fp)
lw $fp, 1($fp)
addi $sp, $sp, 28
jr $ra
```

### 复杂示例（带运算）
C 代码:
```c
int main() {
    int x = input();
    output(x);
    int y = x + 10;
    output(y);
    return 0;
}
```

生成的关键汇编片段:
```asm
input $t0           # 读取 x
sw $t0, -1($fp)
lw $t1, -1($fp)
output $t1          # 输出 x

lw $t2, -1($fp)
addi $t3, $zero, 10
add $t4, $t2, $t3   # y = x + 10
sw $t4, -2($fp)
lw $t5, -2($fp)
output $t5          # 输出 y
```

## 编译流程

```
C 源码: int x = input(); output(x);
    ↓
词法分析: IDENT("input"), LPAREN, RPAREN, ...
    ↓
语法分析: ASTCall(callee="input", args=[])
    ↓
语义分析: 识别为内建函数，返回类型 int
    ↓
IR 生成: {"input", "t0", "", ""}
    ↓
代码生成: input $t0
```

## 特性和限制

### 支持的功能
✅ `input()` - 从键盘读取 32 位整数
✅ `output(int)` - 输出整数的低 8 位到 LCD
✅ 参数类型检查
✅ 多次调用支持
✅ 与其他语言特性（变量、运算、函数调用等）完全集成

### 当前限制
- `output()` 只输出低 8 位（适合 ASCII 字符）
- 不支持格式化输出
- 不支持字符串字面量（可以通过逐字符输出实现）

## 测试结果

所有测试用例均编译成功：
- ✅ test_io_simple.c
- ✅ test_io.c  
- ✅ test_io_calc.c
- ✅ test_io_fibonacci.c

生成的汇编代码正确包含 `input` 和 `output` 指令，寄存器分配正确。

## 使用示例

### 基础 I/O
```c
int x = input();    // 读取输入
output(x);          // 输出到 LCD
```

### 计算器
```c
int a = input();
int b = input();
output(a + b);
```

### 递归函数 + I/O
```c
int fibonacci(int n) {
    if (n < 2) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

int main() {
    int n = input();
    output(fibonacci(n));
    return 0;
}
```

## 总结

成功实现了内建函数机制，使编译器能够将特殊的 C 函数调用直接映射到硬件指令。这种方法：
- 保持了 C 语言的语法一致性
- 避免了对标准库的依赖
- 为硬件特定功能提供了简洁的接口
- 可以轻松扩展到其他内建函数
