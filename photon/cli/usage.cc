/**
 * @file usage.cc
 * @brief usage.h 的实现。
 */

#include "photon/cli/usage.h"

#include <ostream>

namespace photon::cli {

void PrintUsage(std::ostream& out) {
  out << "用法：photon <子命令> [选项]\n"
         "\n"
         "子命令：\n"
         "  scan <目录>    扫描目录并按媒体类型汇总\n"
         "\n"
         "scan 选项：\n"
         "  --no-recursive  只扫描目录的第一层\n"
         "  --all           同时统计无法识别类型的文件\n";
}

}  // namespace photon::cli
