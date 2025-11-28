import os

# --- 项目配置 ---
PROJECT_NAME = "main"
CXX_STANDARD = 17
TEST_DIR = "tests"
SRC_DIR = "src"
MAIN_FILE = "src/main.cpp"  # 主程序入口文件

# --- 递归扫描目录中的所有 .cpp 文件 ---
def find_cpp_files(directory, exclude=None):
    """递归查找目录中的所有 .cpp 文件"""
    cpp_files = []
    exclude = exclude or []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".cpp"):
                relative_path = os.path.relpath(os.path.join(root, file))
                if relative_path not in exclude:
                    cpp_files.append(relative_path)
    return sorted(cpp_files)

# --- 扫描源文件和测试文件 ---
# 排除 main.cpp，因为它包含 main() 函数，会和测试冲突
all_src_files = find_cpp_files(SRC_DIR, exclude=[MAIN_FILE])
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