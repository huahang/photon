/**
 * @file scanner_test.cc
 * @brief scanner.h 的单元测试，使用临时目录构造测试数据。
 */

#include "photon/scan/scanner.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

#include "gtest/gtest.h"
#include "photon/media/media_type.h"

namespace photon::scan {
namespace {

namespace fs = std::filesystem;

/**
 * @brief 为每个用例准备一棵临时目录树的测试夹具。
 */
class ScannerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Bazel 保证 TEST_TMPDIR 存在且用例之间互相隔离。
    const char* tmp = std::getenv("TEST_TMPDIR");
    root_ = fs::path(tmp != nullptr ? tmp : fs::temp_directory_path().string()) /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();
    fs::remove_all(root_);
    fs::create_directories(root_);
  }

  void TearDown() override { fs::remove_all(root_); }

  /**
   * @brief 在临时目录中创建一个带内容的文件，必要时自动建立父目录。
   * @param relative_path 相对于测试根目录的路径。
   * @param content 文件内容，用于让文件大小不为零。
   */
  void WriteFile(const std::string& relative_path, const std::string& content = "photon") {
    const fs::path full = root_ / relative_path;
    fs::create_directories(full.parent_path());
    std::ofstream out(full, std::ios::binary);
    out << content;
  }

  fs::path root_;
};

TEST_F(ScannerTest, 递归收集媒体文件并忽略未知类型) {
  WriteFile("IMG_0001.CR3");
  WriteFile("2026-08-29/IMG_0002.jpg");
  WriteFile("2026-08-29/VIDEO.MOV");
  WriteFile("笔记.txt");

  const auto files = ScanDirectory(root_);
  ASSERT_TRUE(files.ok()) << files.status();
  EXPECT_EQ(files->size(), 3U);
}

TEST_F(ScannerTest, 保留未知类型的选项生效) {
  WriteFile("IMG_0001.CR3");
  WriteFile("笔记.txt");

  ScanOptions options;
  options.include_unknown = true;
  const auto files = ScanDirectory(root_, options);
  ASSERT_TRUE(files.ok()) << files.status();
  EXPECT_EQ(files->size(), 2U);
}

TEST_F(ScannerTest, 非递归模式只看根目录一层) {
  WriteFile("IMG_0001.CR3");
  WriteFile("子目录/IMG_0002.CR3");

  ScanOptions options;
  options.recursive = false;
  const auto files = ScanDirectory(root_, options);
  ASSERT_TRUE(files.ok()) << files.status();
  ASSERT_EQ(files->size(), 1U);
  EXPECT_EQ((*files)[0].path.filename(), "IMG_0001.CR3");
}

TEST_F(ScannerTest, 默认跳过隐藏文件与隐藏目录) {
  WriteFile(".隐藏.jpg");
  WriteFile(".缓存/IMG_0003.jpg");
  WriteFile("IMG_0004.jpg");

  const auto files = ScanDirectory(root_);
  ASSERT_TRUE(files.ok()) << files.status();
  ASSERT_EQ(files->size(), 1U);
  EXPECT_EQ((*files)[0].path.filename(), "IMG_0004.jpg");
}

TEST_F(ScannerTest, 结果按路径排序且携带类型与大小) {
  WriteFile("b.jpg", "12345");
  WriteFile("a.mp4", "1234567890");

  const auto files = ScanDirectory(root_);
  ASSERT_TRUE(files.ok()) << files.status();
  ASSERT_EQ(files->size(), 2U);
  EXPECT_EQ((*files)[0].path.filename(), "a.mp4");
  EXPECT_EQ((*files)[0].type, media::MediaType::kVideo);
  EXPECT_EQ((*files)[0].size_bytes, 10U);
  EXPECT_EQ((*files)[1].path.filename(), "b.jpg");
  EXPECT_EQ((*files)[1].type, media::MediaType::kJpegImage);
  EXPECT_EQ((*files)[1].size_bytes, 5U);
}

TEST_F(ScannerTest, 目录不存在时返回NotFound) {
  const auto files = ScanDirectory(root_ / "不存在");
  ASSERT_FALSE(files.ok());
  EXPECT_EQ(files.status().code(), absl::StatusCode::kNotFound);
}

TEST_F(ScannerTest, 传入普通文件时返回InvalidArgument) {
  WriteFile("IMG_0001.CR3");

  const auto files = ScanDirectory(root_ / "IMG_0001.CR3");
  ASSERT_FALSE(files.ok());
  EXPECT_EQ(files.status().code(), absl::StatusCode::kInvalidArgument);
}

}  // namespace
}  // namespace photon::scan
