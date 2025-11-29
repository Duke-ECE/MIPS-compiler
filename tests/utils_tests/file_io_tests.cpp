/**
 * @file file_io_tests.cpp
 * @brief FileIO 工具类单元测试
 * 
 * 测试覆盖：
 * - 文件读取
 * - 文件写入
 * - Intel HEX 格式生成
 * - MIF 格式生成
 * - 路径工具函数
 */

#include "utils/file_io.hpp"
#include <iostream>
#include <cassert>
#include <cstdio>

using namespace utils;

// ============================================================================
// 测试辅助宏
// ============================================================================

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        std::cerr << "\n断言失败: " << #cond << std::endl; \
        assert(false); \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        std::cerr << "\n断言失败: " << #a << " == " << #b << std::endl; \
        std::cerr << "  实际值: " << (a) << " vs " << (b) << std::endl; \
        assert(false); \
    } \
} while(0)

// ============================================================================
// 文件读写测试
// ============================================================================

TEST(write_and_read_file) {
    const std::string testPath = "/tmp/file_io_test.txt";
    const std::string content = "Hello, World!\nLine 2\nLine 3";
    
    // 写入文件
    auto result = FileIO::writeFile(testPath, content);
    ASSERT_TRUE(result.success);
    
    // 读取文件
    auto readContent = FileIO::readFile(testPath);
    ASSERT_TRUE(readContent.has_value());
    ASSERT_EQ(*readContent, content);
    
    // 清理
    std::remove(testPath.c_str());
}

TEST(read_nonexistent_file) {
    auto content = FileIO::readFile("/tmp/nonexistent_file_12345.txt");
    ASSERT_FALSE(content.has_value());
}

TEST(read_lines) {
    const std::string testPath = "/tmp/file_io_lines_test.txt";
    const std::string content = "Line 1\nLine 2\nLine 3";
    
    // 写入文件
    FileIO::writeFile(testPath, content);
    
    // 读取行
    auto lines = FileIO::readLines(testPath);
    ASSERT_TRUE(lines.has_value());
    ASSERT_EQ(lines->size(), 3u);
    ASSERT_EQ((*lines)[0], "Line 1");
    ASSERT_EQ((*lines)[1], "Line 2");
    ASSERT_EQ((*lines)[2], "Line 3");
    
    // 清理
    std::remove(testPath.c_str());
}

TEST(write_lines) {
    const std::string testPath = "/tmp/file_io_write_lines_test.txt";
    std::vector<std::string> lines = {"First", "Second", "Third"};
    
    // 写入行
    auto result = FileIO::writeLines(testPath, lines);
    ASSERT_TRUE(result.success);
    
    // 读取验证
    auto readLines = FileIO::readLines(testPath);
    ASSERT_TRUE(readLines.has_value());
    ASSERT_EQ(readLines->size(), 3u);
    ASSERT_EQ((*readLines)[0], "First");
    ASSERT_EQ((*readLines)[1], "Second");
    ASSERT_EQ((*readLines)[2], "Third");
    
    // 清理
    std::remove(testPath.c_str());
}

// ============================================================================
// Intel HEX 格式测试
// ============================================================================

TEST(generate_hex_empty) {
    std::vector<uint32_t> code;
    std::string hex = FileIO::generateHex(code);
    
    // 空代码应该只有 EOF 记录
    ASSERT_TRUE(hex.find(":00000001FF") != std::string::npos);
}

TEST(generate_hex_single_word) {
    std::vector<uint32_t> code = {0x31408000};
    std::string hex = FileIO::generateHex(code);
    
    // 应该包含数据记录
    ASSERT_TRUE(hex.find(':') != std::string::npos);
    // 应该包含 EOF 记录
    ASSERT_TRUE(hex.find(":00000001FF") != std::string::npos);
    // 应该包含数据 31408000
    ASSERT_TRUE(hex.find("31408000") != std::string::npos);
}

TEST(generate_hex_multiple_words) {
    std::vector<uint32_t> code = {0x00000000, 0x11111111, 0x22222222};
    std::string hex = FileIO::generateHex(code);
    
    // 计算记录数
    int recordCount = 0;
    for (char c : hex) {
        if (c == ':') recordCount++;
    }
    
    // 固定输出 4096 个数据记录 + 1 个 EOF 记录 = 4097
    ASSERT_EQ(recordCount, 4097);
    
    // 验证数据内容
    ASSERT_TRUE(hex.find("00000000") != std::string::npos);
    ASSERT_TRUE(hex.find("11111111") != std::string::npos);
    ASSERT_TRUE(hex.find("22222222") != std::string::npos);
}

TEST(generate_hex_address_increment) {
    std::vector<uint32_t> code = {0x00000000, 0x11111111};
    std::string hex = FileIO::generateHex(code);
    
    // 第一条记录地址 0000
    ASSERT_TRUE(hex.find(":04000000") != std::string::npos);
    // 第二条记录地址 0004（字节地址）
    ASSERT_TRUE(hex.find(":04000400") != std::string::npos);
}

TEST(write_hex_file) {
    const std::string testPath = "/tmp/file_io_test.hex";
    std::vector<uint32_t> code = {0x12345678, 0xDEADBEEF};
    
    // 写入 HEX 文件
    auto result = FileIO::writeHex(testPath, code);
    ASSERT_TRUE(result.success);
    
    // 读取验证
    auto content = FileIO::readFile(testPath);
    ASSERT_TRUE(content.has_value());
    ASSERT_TRUE(content->find(":00000001FF") != std::string::npos);
    
    // 清理
    std::remove(testPath.c_str());
}

// ============================================================================
// MIF 格式测试
// ============================================================================

TEST(generate_mif_empty) {
    std::vector<uint32_t> code;
    std::string mif = FileIO::generateMif(code);
    
    // 应该包含 MIF 头部
    ASSERT_TRUE(mif.find("DEPTH") != std::string::npos);
    ASSERT_TRUE(mif.find("WIDTH") != std::string::npos);
    ASSERT_TRUE(mif.find("CONTENT") != std::string::npos);
    ASSERT_TRUE(mif.find("BEGIN") != std::string::npos);
    ASSERT_TRUE(mif.find("END") != std::string::npos);
}

TEST(generate_mif_single_word) {
    std::vector<uint32_t> code = {0x12345678};
    std::string mif = FileIO::generateMif(code);
    
    // 应该包含数据
    ASSERT_TRUE(mif.find("12345678") != std::string::npos);
    // 应该包含地址 0000
    ASSERT_TRUE(mif.find("0000") != std::string::npos);
}

TEST(generate_mif_with_depth) {
    std::vector<uint32_t> code = {0x00000000, 0x11111111};
    std::string mif = FileIO::generateMif(code, 256, 32);
    
    // 应该包含指定的深度
    ASSERT_TRUE(mif.find("DEPTH = 256") != std::string::npos);
    ASSERT_TRUE(mif.find("WIDTH = 32") != std::string::npos);
}

TEST(generate_mif_fill_zeros) {
    std::vector<uint32_t> code = {0x12345678};
    std::string mif = FileIO::generateMif(code, 16, 32);
    
    // 应该有填充零的范围
    ASSERT_TRUE(mif.find("00000000") != std::string::npos);
}

TEST(write_mif_file) {
    const std::string testPath = "/tmp/file_io_test.mif";
    std::vector<uint32_t> code = {0x12345678, 0xABCDEF00};
    
    // 写入 MIF 文件
    auto result = FileIO::writeMif(testPath, code);
    ASSERT_TRUE(result.success);
    
    // 读取验证
    auto content = FileIO::readFile(testPath);
    ASSERT_TRUE(content.has_value());
    ASSERT_TRUE(content->find("CONTENT") != std::string::npos);
    
    // 清理
    std::remove(testPath.c_str());
}

// ============================================================================
// 路径工具测试
// ============================================================================

TEST(get_extension) {
    ASSERT_EQ(FileIO::getExtension("test.s"), ".s");
    ASSERT_EQ(FileIO::getExtension("path/to/file.hex"), ".hex");
    ASSERT_EQ(FileIO::getExtension("noext"), "");
    // 注意: .hidden 被视为文件名，不是扩展名（符合 POSIX 隐藏文件约定）
    ASSERT_EQ(FileIO::getExtension(".hidden"), "");
    ASSERT_EQ(FileIO::getExtension(".hidden.txt"), ".txt");
    ASSERT_EQ(FileIO::getExtension("file.tar.gz"), ".gz");
}

TEST(get_file_name) {
    ASSERT_EQ(FileIO::getFileName("test.s"), "test.s");
    ASSERT_EQ(FileIO::getFileName("path/to/file.hex"), "file.hex");
    ASSERT_EQ(FileIO::getFileName("/absolute/path/test.mif"), "test.mif");
}

TEST(get_base_name) {
    ASSERT_EQ(FileIO::getBaseName("test.s"), "test");
    ASSERT_EQ(FileIO::getBaseName("path/to/file.hex"), "file");
    ASSERT_EQ(FileIO::getBaseName("noext"), "noext");
}

TEST(get_directory) {
    ASSERT_EQ(FileIO::getDirectory("path/to/file.s"), "path/to");
    ASSERT_EQ(FileIO::getDirectory("/absolute/path/test.hex"), "/absolute/path");
    ASSERT_EQ(FileIO::getDirectory("file.s"), "");
}

TEST(replace_extension) {
    ASSERT_EQ(FileIO::replaceExtension("test.s", ".hex"), "test.hex");
    ASSERT_EQ(FileIO::replaceExtension("path/to/file.s", ".mif"), "path/to/file.mif");
    ASSERT_EQ(FileIO::replaceExtension("noext", ".hex"), "noext.hex");
}

TEST(join_path) {
    ASSERT_EQ(FileIO::joinPath("path/to", "file.s"), "path/to/file.s");
    ASSERT_EQ(FileIO::joinPath("/absolute", "test.hex"), "/absolute/test.hex");
}

TEST(exists) {
    const std::string testPath = "/tmp/file_io_exists_test.txt";
    
    // 文件不存在
    ASSERT_FALSE(FileIO::exists(testPath));
    
    // 创建文件
    FileIO::writeFile(testPath, "test");
    ASSERT_TRUE(FileIO::exists(testPath));
    
    // 清理
    std::remove(testPath.c_str());
    ASSERT_FALSE(FileIO::exists(testPath));
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "   FileIO 单元测试" << std::endl;
    std::cout << "============================================" << std::endl;
    
    // 文件读写测试
    std::cout << "\n--- 文件读写测试 ---" << std::endl;
    RUN_TEST(write_and_read_file);
    RUN_TEST(read_nonexistent_file);
    RUN_TEST(read_lines);
    RUN_TEST(write_lines);
    
    // Intel HEX 格式测试
    std::cout << "\n--- Intel HEX 格式测试 ---" << std::endl;
    RUN_TEST(generate_hex_empty);
    RUN_TEST(generate_hex_single_word);
    RUN_TEST(generate_hex_multiple_words);
    RUN_TEST(generate_hex_address_increment);
    RUN_TEST(write_hex_file);
    
    // MIF 格式测试
    std::cout << "\n--- MIF 格式测试 ---" << std::endl;
    RUN_TEST(generate_mif_empty);
    RUN_TEST(generate_mif_single_word);
    RUN_TEST(generate_mif_with_depth);
    RUN_TEST(generate_mif_fill_zeros);
    RUN_TEST(write_mif_file);
    
    // 路径工具测试
    std::cout << "\n--- 路径工具测试 ---" << std::endl;
    RUN_TEST(get_extension);
    RUN_TEST(get_file_name);
    RUN_TEST(get_base_name);
    RUN_TEST(get_directory);
    RUN_TEST(replace_extension);
    RUN_TEST(join_path);
    RUN_TEST(exists);
    
    std::cout << "\n============================================" << std::endl;
    std::cout << "   所有测试通过！" << std::endl;
    std::cout << "============================================" << std::endl;
    
    return 0;
}
