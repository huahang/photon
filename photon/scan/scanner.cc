/**
 * @file scanner.cc
 * @brief scanner.h 中目录扫描逻辑的实现。
 */

#include "photon/scan/scanner.h"

#include <algorithm>
#include <filesystem>
#include <system_error>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "photon/media/media_type.h"

namespace photon::scan {
namespace {

namespace fs = std::filesystem;

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
  const auto kOptions = fs::directory_options::skip_permission_denied;

  if (options.recursive) {
    fs::recursive_directory_iterator it(root, kOptions, ec);
    if (ec) {
      return absl::InternalError(absl::StrCat("无法遍历目录：", root.string(), "：", ec.message()));
    }
    // 使用显式迭代而非 range-for，才能对隐藏目录调用 disable_recursion_pending()。
    const fs::recursive_directory_iterator kEnd;
    while (it != kEnd) {
      const fs::directory_entry& entry = *it;
      if (options.skip_hidden && IsHidden(entry.path())) {
        // 隐藏目录整棵子树都不再进入。
        if (entry.is_directory(ec) && !ec) {
          it.disable_recursion_pending();
        }
      } else if (entry.is_regular_file(ec) && !ec) {
        CollectFile(entry.path(), options, &files);
      }
      it.increment(ec);
      if (ec) {
        // 迭代器状态已不可靠，返回此前收集到的结果。
        break;
      }
    }
  } else {
    fs::directory_iterator it(root, kOptions, ec);
    if (ec) {
      return absl::InternalError(absl::StrCat("无法遍历目录：", root.string(), "：", ec.message()));
    }
    for (const fs::directory_entry& entry : it) {
      if (options.skip_hidden && IsHidden(entry.path())) {
        continue;
      }
      if (entry.is_regular_file(ec) && !ec) {
        CollectFile(entry.path(), options, &files);
      }
    }
  }

  std::sort(files.begin(), files.end(),
            [](const MediaFile& lhs, const MediaFile& rhs) { return lhs.path < rhs.path; });
  return files;
}

}  // namespace photon::scan
