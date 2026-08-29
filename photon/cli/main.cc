/**
 * @file main.cc
 * @brief photon 命令行入口。
 *
 * 当前支持的子命令：
 *   - scan <目录>：递归扫描目录，按媒体类型汇总文件数量与体积。
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "photon/media/media_type.h"
#include "photon/scan/scanner.h"

namespace photon::cli {
namespace {

/**
 * @brief 打印命令行用法说明。
 */
void PrintUsage() {
  std::fputs(
      "用法：photon <子命令> [选项]\n"
      "\n"
      "子命令：\n"
      "  scan <目录>    扫描目录并按媒体类型汇总\n"
      "\n"
      "scan 选项：\n"
      "  --no-recursive  只扫描目录的第一层\n"
      "  --all           同时统计无法识别类型的文件\n",
      stderr);
}

/**
 * @brief 把字节数格式化成便于阅读的字符串。
 * @param bytes 字节数。
 * @return 形如 "1.5 GiB" 的字符串。
 */
std::string FormatSize(std::uintmax_t bytes) {
  constexpr std::array<std::string_view, 5> kUnits = {"B", "KiB", "MiB", "GiB", "TiB"};
  auto value = static_cast<double>(bytes);
  size_t unit = 0;
  while (value >= 1024.0 && unit + 1 < kUnits.size()) {
    value /= 1024.0;
    ++unit;
  }
  return unit == 0 ? absl::StrFormat("%d %s", bytes, kUnits[unit])
                   : absl::StrFormat("%.1f %s", value, kUnits[unit]);
}

/**
 * @brief 执行 scan 子命令。
 * @param args scan 之后的全部参数。
 * @return 进程退出码，0 表示成功。
 */
int RunScan(const std::vector<std::string_view>& args) {
  scan::ScanOptions options;
  std::filesystem::path root;
  for (std::string_view arg : args) {
    if (arg == "--no-recursive") {
      options.recursive = false;
    } else if (arg == "--all") {
      options.include_unknown = true;
    } else if (arg.starts_with("-")) {
      absl::FPrintF(stderr, "未知选项：%s\n", arg);
      return 2;
    } else if (root.empty()) {
      root = std::filesystem::path(arg);
    } else {
      std::fputs("scan 只接受一个目录参数\n", stderr);
      return 2;
    }
  }
  if (root.empty()) {
    PrintUsage();
    return 2;
  }

  const absl::StatusOr<std::vector<scan::MediaFile>> files = scan::ScanDirectory(root, options);
  if (!files.ok()) {
    absl::FPrintF(stderr, "扫描失败：%s\n", files.status().message());
    return 1;
  }

  // 以枚举值为键，保证输出顺序稳定。
  std::map<media::MediaType, std::pair<size_t, std::uintmax_t>> summary;
  std::uintmax_t total_bytes = 0;
  for (const scan::MediaFile& file : *files) {
    auto& [count, bytes] = summary[file.type];
    ++count;
    bytes += file.size_bytes;
    total_bytes += file.size_bytes;
  }

  absl::PrintF("扫描目录：%s\n", root.string());
  for (const auto& [type, stats] : summary) {
    absl::PrintF("  %-8s %6d 个   %s\n", media::ToString(type), stats.first,
                 FormatSize(stats.second));
  }
  absl::PrintF("合计：%d 个文件，%s\n", files->size(), FormatSize(total_bytes));
  return 0;
}

}  // namespace
}  // namespace photon::cli

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
      photon::cli::PrintUsage();
      return 2;
    }
    if (args[0] == "scan") {
      return photon::cli::RunScan({args.begin() + 1, args.end()});
    }
    if (args[0] == "--help" || args[0] == "-h" || args[0] == "help") {
      photon::cli::PrintUsage();
      return 0;
    }
    absl::FPrintF(stderr, "未知子命令：%s\n", args[0]);
    photon::cli::PrintUsage();
    return 2;
  } catch (const std::exception& e) {
    absl::FPrintF(stderr, "发生未预期的错误：%s\n", e.what());
    return 1;
  }
}
