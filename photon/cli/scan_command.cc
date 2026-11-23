/**
 * @file scan_command.cc
 * @brief scan_command.h 的实现。
 */

#include "photon/cli/scan_command.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "photon/cli/format_size.h"
#include "photon/cli/usage.h"
#include "photon/media/media_type.h"
#include "photon/scan/scanner.h"

namespace photon::cli {

int RunScan(const std::vector<std::string_view>& args, std::ostream& out, std::ostream& err) {
  scan::ScanOptions options;
  std::filesystem::path root;
  for (std::string_view arg : args) {
    if (arg == "--no-recursive") {
      options.recursive = false;
    } else if (arg == "--all") {
      options.include_unknown = true;
    } else if (arg.starts_with("-")) {
      err << absl::StrFormat("未知选项：%s\n", arg);
      return 2;
    } else if (root.empty()) {
      root = std::filesystem::path(arg);
    } else {
      err << "scan 只接受一个目录参数\n";
      return 2;
    }
  }
  if (root.empty()) {
    PrintUsage(err);
    return 2;
  }

  const absl::StatusOr<std::vector<scan::MediaFile>> files = scan::ScanDirectory(root, options);
  if (!files.ok()) {
    err << absl::StrFormat("扫描失败：%s\n", files.status().message());
    return 1;
  }

  // 以枚举值为键，保证输出顺序稳定，与文件被发现的先后无关。
  std::map<media::MediaType, std::pair<size_t, std::uintmax_t>> summary;
  std::uintmax_t total_bytes = 0;
  for (const scan::MediaFile& file : *files) {
    auto& [count, bytes] = summary[file.type];
    ++count;
    bytes += file.size_bytes;
    total_bytes += file.size_bytes;
  }

  out << absl::StrFormat("扫描目录：%s\n", root.string());
  for (const auto& [type, stats] : summary) {
    out << absl::StrFormat("  %-8s %6d 个   %s\n", media::ToString(type), stats.first,
                           FormatSize(stats.second));
  }
  out << absl::StrFormat("合计：%d 个文件，%s\n", files->size(), FormatSize(total_bytes));
  return 0;
}

}  // namespace photon::cli
