/**
 * @file media_type.cc
 * @brief media_type.h 中扩展名识别逻辑的实现。
 */

#include "photon/media/media_type.h"

#include <filesystem>
#include <string>
#include <string_view>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/ascii.h"

namespace photon::media {
namespace {

/**
 * @brief 扩展名到媒体类型的映射表。
 *
 * 键为不含前导点、且已转成小写的扩展名。新增机型或格式时在此处补充。
 *
 * @return 进程内共享的静态映射表。
 */
const absl::flat_hash_map<std::string_view, MediaType>& ExtensionTable() {
  static const auto* table = new absl::flat_hash_map<std::string_view, MediaType>{
      // 各厂商的 RAW 格式。
      {"cr2", MediaType::kRawImage},  // Canon
      {"cr3", MediaType::kRawImage},  // Canon
      {"crw", MediaType::kRawImage},  // Canon（早期机型）
      {"nef", MediaType::kRawImage},  // Nikon
      {"nrw", MediaType::kRawImage},  // Nikon
      {"arw", MediaType::kRawImage},  // Sony
      {"srf", MediaType::kRawImage},  // Sony
      {"sr2", MediaType::kRawImage},  // Sony
      {"raf", MediaType::kRawImage},  // Fujifilm
      {"orf", MediaType::kRawImage},  // Olympus / OM System
      {"rw2", MediaType::kRawImage},  // Panasonic
      {"pef", MediaType::kRawImage},  // Pentax
      {"dng", MediaType::kRawImage},  // Adobe 通用 RAW
      {"raw", MediaType::kRawImage},  // 通用后缀
      {"3fr", MediaType::kRawImage},  // Hasselblad
      {"iiq", MediaType::kRawImage},  // Phase One
      {"gpr", MediaType::kRawImage},  // GoPro
      // 常规图像。
      {"jpg", MediaType::kJpegImage},
      {"jpeg", MediaType::kJpegImage},
      {"jpe", MediaType::kJpegImage},
      {"heic", MediaType::kHeifImage},
      {"heif", MediaType::kHeifImage},
      {"avif", MediaType::kHeifImage},  // 与 HEIF 同属 ISOBMFF 容器
      {"png", MediaType::kPngImage},
      // 视频。
      {"mp4", MediaType::kVideo},
      {"mov", MediaType::kVideo},
      {"m4v", MediaType::kVideo},
      {"avi", MediaType::kVideo},
      {"mts", MediaType::kVideo},
      {"m2ts", MediaType::kVideo},
      {"mxf", MediaType::kVideo},
      {"insv", MediaType::kVideo},  // Insta360
      // 附属文件。
      {"xmp", MediaType::kSidecar},
      {"aae", MediaType::kSidecar},  // Apple 照片编辑记录
      {"thm", MediaType::kSidecar},  // 视频缩略图
      {"lrv", MediaType::kSidecar},  // GoPro 低码率代理
  };
  return *table;
}

}  // namespace

MediaType DetectByExtension(const std::filesystem::path& path) {
  const std::string extension = path.extension().string();
  if (extension.size() < 2 || extension.front() != '.') {
    return MediaType::kUnknown;
  }
  const std::string key = absl::AsciiStrToLower(std::string_view(extension).substr(1));
  const auto& table = ExtensionTable();
  const auto it = table.find(key);
  return it == table.end() ? MediaType::kUnknown : it->second;
}

bool IsImage(MediaType type) {
  switch (type) {
    case MediaType::kRawImage:
    case MediaType::kJpegImage:
    case MediaType::kHeifImage:
    case MediaType::kPngImage:
      return true;
    default:
      return false;
  }
}

bool IsVideo(MediaType type) { return type == MediaType::kVideo; }

std::string_view ToString(MediaType type) {
  switch (type) {
    case MediaType::kRawImage:
      return "raw";
    case MediaType::kJpegImage:
      return "jpeg";
    case MediaType::kHeifImage:
      return "heif";
    case MediaType::kPngImage:
      return "png";
    case MediaType::kVideo:
      return "video";
    case MediaType::kSidecar:
      return "sidecar";
    case MediaType::kUnknown:
      return "unknown";
  }
  return "unknown";
}

}  // namespace photon::media
