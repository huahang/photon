/**
 * @file scanner.h
 * @brief 遍历目录、收集其中媒体文件的扫描器。
 */

#ifndef PHOTON_SCAN_SCANNER_H_
#define PHOTON_SCAN_SCANNER_H_

#include <cstdint>
#include <filesystem>
#include <vector>

#include "absl/status/statusor.h"
#include "photon/media/media_type.h"

namespace photon::scan {

/**
 * @brief 扫描过程中发现的单个媒体文件。
 */
struct MediaFile {
  std::filesystem::path path;  ///< 文件的绝对路径
  media::MediaType type;       ///< 按扩展名识别出的媒体类型
  std::uintmax_t size_bytes;   ///< 文件大小，单位为字节
};

/**
 * @brief 扫描行为的可调选项。
 */
struct ScanOptions {
  bool recursive = true;         ///< 是否递归进入子目录
  bool skip_hidden = true;       ///< 是否跳过以点号开头的文件与目录
  bool include_unknown = false;  ///< 是否保留无法识别类型的文件
};

/**
 * @brief 扫描目录并返回其中的媒体文件列表。
 *
 * 遍历过程中遇到的单个条目错误（如权限不足、符号链接失效）会被跳过，
 * 不会中断整次扫描；只有根目录本身不可用时才返回错误。
 * 返回结果按路径字典序排序，以保证多次运行的输出稳定。
 *
 * @param root 待扫描的目录。
 * @param options 扫描选项，默认递归、跳过隐藏文件、丢弃未知类型。
 * @return 成功时返回媒体文件列表；根目录不存在或不是目录时返回错误状态。
 */
absl::StatusOr<std::vector<MediaFile>> ScanDirectory(const std::filesystem::path& root,
                                                     const ScanOptions& options = ScanOptions{});

}  // namespace photon::scan

#endif  // PHOTON_SCAN_SCANNER_H_
