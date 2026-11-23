/**
 * @file usage_test.cc
 * @brief PrintUsage 的单元测试。
 */

#include "photon/cli/usage.h"

#include <sstream>

#include "gtest/gtest.h"

namespace photon::cli {
namespace {

TEST(UsageTest, PrintsTheFullHelpText) {
  std::ostringstream out;
  PrintUsage(out);
  EXPECT_EQ(out.str(),
            "用法：photon <子命令> [选项]\n"
            "\n"
            "子命令：\n"
            "  scan <目录>    扫描目录并按媒体类型汇总\n"
            "\n"
            "scan 选项：\n"
            "  --no-recursive  只扫描目录的第一层\n"
            "  --all           同时统计无法识别类型的文件\n");
}

}  // namespace
}  // namespace photon::cli
