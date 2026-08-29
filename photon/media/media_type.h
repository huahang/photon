/**
 * @file media_type.h
 * @brief 相机媒体文件类型的定义与识别。
 */

#ifndef PHOTON_MEDIA_MEDIA_TYPE_H_
#define PHOTON_MEDIA_MEDIA_TYPE_H_

#include <cstdint>
#include <filesystem>
#include <string_view>

namespace photon::media {

/**
 * @brief 相机产出的媒体文件类型。
 *
 * 显式指定 1 字节的底层类型：该枚举会大量存放在 MediaFile 数组中，
 * 缩小它可以减少扫描大目录时的内存占用。
 */
enum class MediaType : std::uint8_t {
  kUnknown = 0,  ///< 无法识别的类型
  kRawImage,     ///< 相机 RAW 原始图像，如 CR3 / NEF / ARW / DNG
  kJpegImage,    ///< JPEG 图像
  kHeifImage,    ///< HEIF / HEIC 图像
  kPngImage,     ///< PNG 图像
  kVideo,        ///< 视频，如 MP4 / MOV
  kSidecar,      ///< 附属文件，如 XMP / AAE / THM
};

/**
 * @brief 根据文件扩展名识别媒体类型。
 *
 * 只依据扩展名判断，不读取文件内容，因此速度快但可能被伪造的扩展名欺骗；
 * 需要严格判定时应另行读取文件头。扩展名大小写不敏感。
 *
 * @param path 文件路径，可为相对路径或绝对路径。
 * @return 识别出的媒体类型；无法识别时返回 MediaType::kUnknown。
 */
MediaType DetectByExtension(const std::filesystem::path& path);

/**
 * @brief 判断是否为图像类型（RAW / JPEG / HEIF / PNG）。
 * @param type 媒体类型。
 * @return 是图像返回 true，否则返回 false。
 */
bool IsImage(MediaType type);

/**
 * @brief 判断是否为视频类型。
 * @param type 媒体类型。
 * @return 是视频返回 true，否则返回 false。
 */
bool IsVideo(MediaType type);

/**
 * @brief 返回媒体类型的稳定短名称，用于日志与命令行输出。
 * @param type 媒体类型。
 * @return 形如 "raw"、"jpeg" 的静态字符串，调用方无需释放。
 */
std::string_view ToString(MediaType type);

}  // namespace photon::media

#endif  // PHOTON_MEDIA_MEDIA_TYPE_H_
