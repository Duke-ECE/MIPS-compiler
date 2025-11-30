# MIPS-compiler
MIPS ISA compiler kit supports DE2-115 series FPGA development boards.

## 最新更新

### 内建 I/O 函数支持 (Built-in I/O Functions)

编译器现在支持两个特殊的内建函数用于硬件 I/O 操作：

- **`input()`** - 从键盘读取 32 位整数
- **`output(int)`** - 将整数的低 8 位输出到 LCD

这些函数直接映射到 MIPS ISA 的 I 型指令（opcode=01110 和 01111），用于与 FPGA 上的键盘和 LCD 硬件交互。

**使用示例**:
```c
int main() {
    int x = input();      // 从键盘读取
    int y = x + 10;       // 计算
    output(y);            // 输出到 LCD
    return 0;
}
```

详细文档：
- [内建函数使用指南](docs/builtin_io_functions.md)
- [实现细节](docs/builtin_io_implementation.md)
