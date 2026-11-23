/**
 * @file scan_command_test.cc
 * @brief RunScan 的单元测试，覆盖参数解析、失败路径与汇总输出。
 */

#include "photon/cli/scan_command.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

namespace photon::cli {
namespace {

namespace fs = std::filesystem;

/**
 * @brief 一次 RunScan 调用的退出码与两路输出。
 */
struct ScanResult {
  int code;         ///< 进程退出码
  std::string out;  ///< 汇总输出
  std::string err;  ///< 错误与用法输出
};

/**
 * @brief 为每个用例准备临时目录，并把 RunScan 的输出收进字符串。
 */
class ScanCommandTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const char* tmp = std::getenv("TEST_TMPDIR");
    root_ = fs::path(tmp != nullptr ? tmp : fs::temp_directory_path().string()) /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();
    fs::remove_all(root_);
    fs::create_directories(root_);
    root_text_ = root_.string();
  }

  void TearDown() override { fs::remove_all(root_); }

  /**
   * @brief 在临时目录中创建一个指定内容的文件。
   * @param relative_path 相对于测试根目录的路径。
   * @param content 文件内容，其长度即文件大小。
   */
  void WriteFile(const std::string& relative_path, const std::string& content) {
    const fs::path full = root_ / relative_path;
    fs::create_directories(full.parent_path());
    std::ofstream out(full, std::ios::binary);
    out << content;
  }

  fs::path root_;
  std::string root_text_;
};

/**
 * @brief 用给定参数执行 scan，并捕获两路输出。
 * @param args scan 之后的参数。其中的 string_view 必须在调用期间保持有效。
 * @return 退出码与输出。
 */
[[nodiscard]] ScanResult Invoke(const std::vector<std::string_view>& args) {
  std::ostringstream out;
  std::ostringstream err;
  const int code = RunScan(args, out, err);
  return ScanResult{.code = code, .out = out.str(), .err = err.str()};
}

TEST_F(ScanCommandTest, MissingDirectoryPrintsUsage) {
  const ScanResult result = Invoke({});
  EXPECT_EQ(result.code, 2);
  EXPECT_TRUE(result.out.empty());
  EXPECT_NE(result.err.find("用法：photon"), std::string::npos);
  EXPECT_NE(result.err.find("--no-recursive"), std::string::npos);
  EXPECT_NE(result.err.find("--all"), std::string::npos);
}

TEST_F(ScanCommandTest, EmptyPathIsTreatedAsMissingDirectory) {
  const ScanResult result = Invoke({""});
  EXPECT_EQ(result.code, 2);
  EXPECT_TRUE(result.out.empty());
  EXPECT_NE(result.err.find("用法：photon"), std::string::npos);
}

TEST_F(ScanCommandTest, UnknownOption) {
  const ScanResult result = Invoke({"--recursive", root_text_});
  EXPECT_EQ(result.code, 2);
  EXPECT_TRUE(result.out.empty());
  EXPECT_EQ(result.err, "未知选项：--recursive\n");
}

TEST_F(ScanCommandTest, SingleDashIsAnUnknownOption) {
  const ScanResult result = Invoke({"-"});
  EXPECT_EQ(result.code, 2);
  EXPECT_EQ(result.err, "未知选项：-\n");
}

TEST_F(ScanCommandTest, RejectsASecondDirectory) {
  const ScanResult result = Invoke({root_text_, root_text_});
  EXPECT_EQ(result.code, 2);
  EXPECT_TRUE(result.out.empty());
  EXPECT_EQ(result.err, "scan 只接受一个目录参数\n");
}

TEST_F(ScanCommandTest, SecondDirectoryAfterFlagsIsStillRejected) {
  const ScanResult result = Invoke({"--all", root_text_, "--no-recursive", root_text_});
  EXPECT_EQ(result.code, 2);
  EXPECT_EQ(result.err, "scan 只接受一个目录参数\n");
}

TEST_F(ScanCommandTest, FlagsWithoutDirectoryPrintUsage) {
  const ScanResult result = Invoke({"--all", "--no-recursive"});
  EXPECT_EQ(result.code, 2);
  EXPECT_TRUE(result.out.empty());
  EXPECT_NE(result.err.find("scan <目录>"), std::string::npos);
}

TEST_F(ScanCommandTest, MissingDirectoryReportsNotFound) {
  const std::string missing = (root_ / "不存在").string();
  const ScanResult result = Invoke({missing});
  EXPECT_EQ(result.code, 1);
  EXPECT_TRUE(result.out.empty());
  EXPECT_NE(result.err.find("扫描失败："), std::string::npos);
  EXPECT_NE(result.err.find("目录不存在："), std::string::npos);
  EXPECT_NE(result.err.find(missing), std::string::npos);
}

TEST_F(ScanCommandTest, RegularFileReportsInvalidArgument) {
  WriteFile("IMG_0001.CR3", "raw");
  const std::string file = (root_ / "IMG_0001.CR3").string();
  const ScanResult result = Invoke({file});
  EXPECT_EQ(result.code, 1);
  EXPECT_TRUE(result.out.empty());
  EXPECT_NE(result.err.find("路径不是目录："), std::string::npos);
  EXPECT_NE(result.err.find(file), std::string::npos);
}

TEST_F(ScanCommandTest, EmptyDirectoryPrintsZeroTotal) {
  const ScanResult result = Invoke({root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ + "\n合计：0 个文件，0 B\n");
}

TEST_F(ScanCommandTest, SumsFilesOfTheSameType) {
  WriteFile("a.jpg", "12");
  WriteFile("b.jpeg", "345");
  WriteFile("c.JPG", "6");

  const ScanResult result = Invoke({root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  jpeg          3 个   6 B\n"
                            "合计：3 个文件，6 B\n");
}

TEST_F(ScanCommandTest, SummaryFollowsMediaTypeOrderNotPathOrder) {
  WriteFile("c.mov", "vvvvv");
  WriteFile("a.xmp", "xx");
  WriteFile("b.CR3", "r");

  const ScanResult result = Invoke({root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  raw           1 个   1 B\n"
                            "  video         1 个   5 B\n"
                            "  sidecar       1 个   2 B\n"
                            "合计：3 个文件，8 B\n");
}

TEST_F(ScanCommandTest, FormatsSizesInsideTheSummary) {
  WriteFile("clip.mp4", std::string(1536, 'v'));
  WriteFile("still.jpg", std::string(1024, 'j'));

  const ScanResult result = Invoke({root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  jpeg          1 个   1.0 KiB\n"
                            "  video         1 个   1.5 KiB\n"
                            "合计：2 个文件，2.5 KiB\n");
}

TEST_F(ScanCommandTest, ReportsEveryRecognizedFamily) {
  WriteFile("a.cr3", "r");
  WriteFile("b.jpg", "jj");
  WriteFile("c.heic", "hhh");
  WriteFile("d.png", "pppp");
  WriteFile("e.mov", "vvvvv");
  WriteFile("f.xmp", "xxxxxx");
  WriteFile("notes.txt", "unknown-text");

  const ScanResult result = Invoke({root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  raw           1 个   1 B\n"
                            "  jpeg          1 个   2 B\n"
                            "  heif          1 个   3 B\n"
                            "  png           1 个   4 B\n"
                            "  video         1 个   5 B\n"
                            "  sidecar       1 个   6 B\n"
                            "合计：6 个文件，21 B\n");
}

TEST_F(ScanCommandTest, IncludeUnknownCountsUnrecognizedFiles) {
  WriteFile("a.jpg", "jj");
  WriteFile("notes.txt", "txt");

  const ScanResult without = Invoke({root_text_});
  EXPECT_EQ(without.code, 0);
  EXPECT_EQ(without.out.find("unknown"), std::string::npos);
  EXPECT_NE(without.out.find("合计：1 个文件，2 B\n"), std::string::npos);

  const ScanResult with = Invoke({"--all", root_text_});
  EXPECT_EQ(with.code, 0);
  EXPECT_TRUE(with.err.empty());
  EXPECT_EQ(with.out, "扫描目录：" + root_text_ +
                          "\n"
                          "  unknown       1 个   3 B\n"
                          "  jpeg          1 个   2 B\n"
                          "合计：2 个文件，5 B\n");
}

TEST_F(ScanCommandTest, NonRecursiveScansOnlyTopLevel) {
  WriteFile("top.jpg", "top");
  WriteFile("album/nested.CR3", "raw-file");

  const ScanResult result = Invoke({root_text_, "--no-recursive"});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  jpeg          1 个   3 B\n"
                            "合计：1 个文件，3 B\n");
}

TEST_F(ScanCommandTest, SkipsHiddenFilesAndDirectoriesByDefault) {
  WriteFile(".hidden.jpg", "hide");
  WriteFile(".cache/IMG_0003.jpg", "cached");
  WriteFile("visible.png", "ok");

  const ScanResult result = Invoke({"--all", root_text_});
  EXPECT_EQ(result.code, 0);
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  png           1 个   2 B\n"
                            "合计：1 个文件，2 B\n");
}

TEST_F(ScanCommandTest, FlagsMayAppearOnEitherSideOfTheDirectory) {
  WriteFile("top.jpg", "jj");
  WriteFile("nested/note.txt", "txt");
  WriteFile("nested/pic.heic", "heif");

  const ScanResult result = Invoke({"--no-recursive", root_text_, "--all"});
  EXPECT_EQ(result.code, 0);
  EXPECT_TRUE(result.err.empty());
  EXPECT_EQ(result.out, "扫描目录：" + root_text_ +
                            "\n"
                            "  jpeg          1 个   2 B\n"
                            "合计：1 个文件，2 B\n");
}

}  // namespace
}  // namespace photon::cli
