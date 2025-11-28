import os

# --- 项目配置 ---
SRC_FILES = ["src/example.cpp"]   # 公共源文件，会附加到每个测试可执行文件
MAIN_TARGET = "src/main.cpp"      # 主程序
PROJECT_NAME = "main"
CXX_STANDARD = 17
TEST_DIR = "tests"

# --- 递归扫描 tests 目录 ---
def find_test_files(directory):
    """递归查找目录中的所有 .cpp 文件"""
    test_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".cpp"):
                # 获取相对于项目根目录的路径
                relative_path = os.path.relpath(os.path.join(root, file))
                test_files.append(relative_path)
    return sorted(test_files)  # 保证顺序一致

tests = find_test_files(TEST_DIR)

# --- 拼接 CMakeLists 内容 ---
lines = [
    "cmake_minimum_required(VERSION 3.10)",
    f"project({PROJECT_NAME})",
    f"set(CMAKE_CXX_STANDARD {CXX_STANDARD})",
    "",
    "include_directories(include)",
    "",
    "enable_testing()"
]

# --- 添加测试 ---
for test_path in tests:
    # 生成可执行文件名：替换路径分隔符为下划线，去掉扩展名
    name = os.path.splitext(test_path.replace("/", "_").replace("\\", "_"))[0]
    srcs = " ".join([test_path] + SRC_FILES)
    lines.append(f"add_executable({name} {srcs})")
    lines.append(f"add_test(NAME {name} COMMAND {name})")
    lines.append("")

# --- 添加主程序 ---
main_srcs = " ".join([MAIN_TARGET] + SRC_FILES)
lines.append(f"add_executable({PROJECT_NAME} {main_srcs})")
lines.append("")

# --- 检查并创建/更新 CMakeLists.txt ---
cmake_file = "CMakeLists.txt"
file_exists = os.path.exists(cmake_file)

cmake_content = "\n".join(lines)
with open(cmake_file, "w", encoding="utf-8") as f:
    f.write(cmake_content)

if file_exists:
    print("✅ CMakeLists.txt 已更新")
else:
    print("✅ CMakeLists.txt 已创建")

print(f"包含以下测试：")
for t in tests:
    print("  -", t)
