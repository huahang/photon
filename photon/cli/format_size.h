/**
 * @file format_size.h
 * @brief 把字节数格式化成便于阅读的字符串。
 */

#ifndef PHOTON_CLI_FORMAT_SIZE_H_
#define PHOTON_CLI_FORMAT_SIZE_H_

#include <cstdint>
#include <string>

namespace photon::cli {

/**
 * @brief 把字节数格式化成便于阅读的字符串。
 *
 * 以 1024 为进位，单位依次为 B、KiB、MiB、GiB、TiB。不足 1 KiB 时按整数打印，
 * 更大的值保留一位小数。超过 1 TiB 后不再继续进位。进位发生在四舍五入之前，
 * 因此略小于进位边界的值可能显示成下一级单位的 1024.0。
 *
 * @param bytes 字节数。
 * @return 形如 "1.5 GiB" 的字符串。
 */
std::string FormatSize(std::uintmax_t bytes);

}  // namespace photon::cli

#endif  // PHOTON_CLI_FORMAT_SIZE_H_
