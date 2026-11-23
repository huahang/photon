/**
 * @file main.cc
 * @brief photon 命令行入口。
 *
 * 当前支持的子命令：
 *   - scan <目录>：递归扫描目录，按媒体类型汇总文件数量与体积。
 */

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "absl/strings/str_format.h"
#include "photon/cli/scan_command.h"
#include "photon/cli/usage.h"

/**
 * @brief 程序入口，负责分发子命令。
 * @param argc 参数个数。
 * @param argv 参数数组。
 * @return 进程退出码。
 */
int main(int argc, char** argv) {
  // 本项目自身用 absl::Status 表达可预期的失败，不抛异常；但标准库仍可能抛
  // （内存不足、路径编码非法等）。这里兜底，避免直接 terminate 而不给用户任何提示。
  try {
    const std::vector<std::string_view> args(argv + 1, argv + argc);
    if (args.empty()) {
      photon::cli::PrintUsage(std::cerr);
      return 2;
    }
    if (args[0] == "scan") {
      return photon::cli::RunScan({args.begin() + 1, args.end()}, std::cout, std::cerr);
    }
    if (args[0] == "--help" || args[0] == "-h" || args[0] == "help") {
      photon::cli::PrintUsage(std::cerr);
      return 0;
    }
    absl::FPrintF(stderr, "未知子命令：%s\n", args[0]);
    photon::cli::PrintUsage(std::cerr);
    return 2;
  } catch (const std::exception& e) {
    absl::FPrintF(stderr, "发生未预期的错误：%s\n", e.what());
    return 1;
  }
}
