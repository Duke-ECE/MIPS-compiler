import os
import re

# --- 项目配置 ---
PROJECT_NAME = "main"
CXX_STANDARD = 17
TEST_DIR = "tests"
SRC_DIR = "src"
MAIN_FILE = "src/main.cpp"  # 主程序入口文件
MAIN_DIR = "src/main"       # 包含 main() 函数的入口文件目录

# --- 递归扫描目录中的所有 .cpp 文件 ---
def find_cpp_files(directory, exclude=None, exclude_dirs=None):
    """递归查找目录中的所有 .cpp 文件"""
    cpp_files = []
    exclude = exclude or []
    exclude_dirs = exclude_dirs or []
    for root, dirs, files in os.walk(directory):
        # 跳过排除的目录
        if any(os.path.relpath(root).startswith(d) for d in exclude_dirs):
            continue
        for file in files:
            if file.endswith(".cpp"):
                relative_path = os.path.relpath(os.path.join(root, file))
                if relative_path not in exclude:
                    cpp_files.append(relative_path)
    return sorted(cpp_files)

# --- 检查文件是否包含有效的 main 函数 ---
def has_valid_main(file_path):
    """检查文件是否包含未被注释的 main 函数"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 移除所有单行注释
        content = re.sub(r'//.*', '', content)
        
        # 移除所有多行注释
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
        
        # 检查是否包含 main 函数
        # 匹配 int main( 或 int main (
        if re.search(r'\bint\s+main\s*\(', content):
            return True
        
        return False
    except Exception as e:
        print(f"⚠️  警告：无法读取文件 {file_path}: {e}")
        return False

# --- 扫描源文件和测试文件 ---
# 排除 main.cpp 和 src/main/ 目录，因为它们包含 main() 函数，会和测试冲突
all_src_files = find_cpp_files(SRC_DIR, exclude=[MAIN_FILE], exclude_dirs=[MAIN_DIR])
test_files = find_cpp_files(TEST_DIR)

# --- 拼接 CMakeLists 内容 ---
lines = [
    "cmake_minimum_required(VERSION 3.10)",
    f"project({PROJECT_NAME})",
    f"set(CMAKE_CXX_STANDARD {CXX_STANDARD})",
    "",
    "include_directories(include)",
    "",
    "enable_testing()",
    ""
]

# --- 添加测试 ---
for test_path in test_files:
    name = os.path.splitext(test_path.replace("/", "_").replace("\\", "_"))[0]
    srcs = " ".join([test_path] + all_src_files)
    lines.append(f"add_executable({name} {srcs})")
    lines.append(f"add_test(NAME {name} COMMAND {name})")
    lines.append("")

# --- 添加主程序 ---
main_srcs = " ".join([MAIN_FILE] + all_src_files)
lines.append(f"add_executable({PROJECT_NAME} {main_srcs})")
lines.append("")

# --- 添加 src/main/ 目录下的入口程序 ---
if os.path.isdir(MAIN_DIR):
    main_entry_files = find_cpp_files(MAIN_DIR)
    valid_entry_files = []
    for entry_file in main_entry_files:
        # 检查文件是否包含有效的 main 函数
        if has_valid_main(entry_file):
            # 从文件名生成可执行文件名（如 assembler_main.cpp -> assembler）
            base_name = os.path.basename(entry_file)
            exe_name = os.path.splitext(base_name)[0].replace("_main", "")
            entry_srcs = " ".join([entry_file] + all_src_files)
            lines.append(f"add_executable({exe_name} {entry_srcs})")
            lines.append("")
            valid_entry_files.append((exe_name, entry_file))
        else:
            print(f"⚠️  跳过 {entry_file}（未找到有效的 main 函数）")

# --- 写入 CMakeLists.txt ---
cmake_file = "CMakeLists.txt"
file_exists = os.path.exists(cmake_file)

cmake_content = "\n".join(lines)
with open(cmake_file, "w", encoding="utf-8") as f:
    f.write(cmake_content)

if file_exists:
    print("✅ CMakeLists.txt 已更新")
else:
    print("✅ CMakeLists.txt 已创建")

print(f"\n包含的源文件 ({len(all_src_files)} 个)：")
for s in all_src_files:
    print("  -", s)

print(f"\n包含的测试 ({len(test_files)} 个)：")
for t in test_files:
    print("  -", t)

# 打印入口程序
if os.path.isdir(MAIN_DIR):
    main_entry_files = find_cpp_files(MAIN_DIR)
    if main_entry_files or valid_entry_files:
        total_programs = 1 + len(valid_entry_files)
        print(f"\n包含的入口程序 ({total_programs} 个)：")
        print(f"  - {PROJECT_NAME} (from {MAIN_FILE})")
        if 'valid_entry_files' in locals():
            for exe_name, entry_file in valid_entry_files:
                print(f"  - {exe_name} (from {entry_file})")