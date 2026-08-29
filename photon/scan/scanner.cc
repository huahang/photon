/**
 * @file scanner.cc
 * @brief scanner.h 中目录扫描逻辑的实现。
 */

#include "photon/scan/scanner.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "photon/media/media_type.h"

namespace photon::scan {
namespace {

namespace fs = std::filesystem;

/// 遍历时跳过无权限访问的目录，而不是让标准库抛出异常。
constexpr auto kIterationOptions = fs::directory_options::skip_permission_denied;

/**
 * @brief 判断目录项是否为隐藏条目（名字以点号开头）。
 * @param path 目录项路径。
 * @return 隐藏返回 true，否则返回 false。
 */
bool IsHidden(const fs::path& path) {
  const std::string name = path.filename().string();
  return !name.empty() && name.front() == '.';
}

/**
 * @brief 按扫描选项决定是否收录某个常规文件，并在收录时追加到结果中。
 * @param path 文件路径。
 * @param options 扫描选项。
 * @param[out] out 收集结果的容器。
 */
void CollectFile(const fs::path& path, const ScanOptions& options, std::vector<MediaFile>* out) {
  const media::MediaType type = media::DetectByExtension(path);
  if (type == media::MediaType::kUnknown && !options.include_unknown) {
    return;
  }
  std::error_code ec;
  const std::uintmax_t size = fs::file_size(path, ec);
  out->push_back(MediaFile{
      .path = path,
      .type = type,
      .size_bytes = ec ? 0 : size,
  });
}

/**
 * @brief 处理单个目录项：该收录的收录，并告知调用方是否应跳过它的整棵子树。
 *
 * 递归与非递归两种遍历共用此函数，以免两处逻辑各自演化。
 *
 * @param entry 目录项。
 * @param options 扫描选项。
 * @param[out] out 收集结果的容器。
 * @return 该项是需要整棵跳过的隐藏目录时返回 true。
 */
bool HandleEntry(const fs::directory_entry& entry, const ScanOptions& options,
                 std::vector<MediaFile>* out) {
  std::error_code ec;
  if (options.skip_hidden && IsHidden(entry.path())) {
    return entry.is_directory(ec) && !ec;
  }
  if (entry.is_regular_file(ec) && !ec) {
    CollectFile(entry.path(), options, out);
  }
  return false;
}

/**
 * @brief 递归遍历目录树。
 * @param root 根目录。
 * @param options 扫描选项。
 * @param[out] out 收集结果的容器。
 * @return 根目录无法打开时返回错误，其余情况返回 OK。
 */
absl::Status ScanRecursive(const fs::path& root, const ScanOptions& options,
                           std::vector<MediaFile>* out) {
  std::error_code ec;
  fs::recursive_directory_iterator it(root, kIterationOptions, ec);
  if (ec) {
    return absl::InternalError(absl::StrCat("无法遍历目录：", root.string(), "：", ec.message()));
  }
  // 使用显式迭代而非 range-for，才能对隐藏目录调用 disable_recursion_pending()。
  const fs::recursive_directory_iterator kEnd;
  while (it != kEnd) {
    if (HandleEntry(*it, options, out)) {
      it.disable_recursion_pending();
    }
    it.increment(ec);
    if (ec) {
      // 迭代器状态已不可靠，保留此前收集到的结果。
      break;
    }
  }
  return absl::OkStatus();
}

/**
 * @brief 只遍历目录的第一层。
 * @param root 根目录。
 * @param options 扫描选项。
 * @param[out] out 收集结果的容器。
 * @return 根目录无法打开时返回错误，其余情况返回 OK。
 */
absl::Status ScanFlat(const fs::path& root, const ScanOptions& options,
                      std::vector<MediaFile>* out) {
  std::error_code ec;
  fs::directory_iterator it(root, kIterationOptions, ec);
  if (ec) {
    return absl::InternalError(absl::StrCat("无法遍历目录：", root.string(), "：", ec.message()));
  }
  for (const fs::directory_entry& entry : it) {
    HandleEntry(entry, options, out);
  }
  return absl::OkStatus();
}

}  // namespace

absl::StatusOr<std::vector<MediaFile>> ScanDirectory(const fs::path& root,
                                                     const ScanOptions& options) {
  std::error_code ec;
  if (!fs::exists(root, ec) || ec) {
    return absl::NotFoundError(absl::StrCat("目录不存在：", root.string()));
  }
  if (!fs::is_directory(root, ec) || ec) {
    return absl::InvalidArgumentError(absl::StrCat("路径不是目录：", root.string()));
  }

  std::vector<MediaFile> files;
  // 此处不加 const：加了会阻止 return 时的自动 move（performance-no-automatic-move）。
  absl::Status status =
      options.recursive ? ScanRecursive(root, options, &files) : ScanFlat(root, options, &files);
  if (!status.ok()) {
    return status;
  }

  std::ranges::sort(files,
                    [](const MediaFile& lhs, const MediaFile& rhs) { return lhs.path < rhs.path; });
  return files;
}

}  // namespace photon::scan
