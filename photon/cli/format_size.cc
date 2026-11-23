/**
 * @file format_size.cc
 * @brief format_size.h 的实现。
 */

#include "photon/cli/format_size.h"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include "absl/strings/str_format.h"

namespace photon::cli {
namespace {

constexpr std::array<std::string_view, 5> kUnits = {"B", "KiB", "MiB", "GiB", "TiB"};

}  // namespace

std::string FormatSize(std::uintmax_t bytes) {
  auto value = static_cast<double>(bytes);
  size_t unit = 0;
  while (value >= 1024.0 && unit + 1 < kUnits.size()) {
    value /= 1024.0;
    ++unit;
  }
  return unit == 0 ? absl::StrFormat("%d %s", bytes, kUnits[unit])
                   : absl::StrFormat("%.1f %s", value, kUnits[unit]);
}

}  // namespace photon::cli
