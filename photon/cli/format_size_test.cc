/**
 * @file format_size_test.cc
 * @brief FormatSize 的单元测试，覆盖进位边界与四舍五入。
 */

#include "photon/cli/format_size.h"

#include <cstdint>

#include "gtest/gtest.h"

namespace photon::cli {
namespace {

constexpr std::uintmax_t kKiB = 1024;
constexpr std::uintmax_t kMiB = kKiB * 1024;
constexpr std::uintmax_t kGiB = kMiB * 1024;
constexpr std::uintmax_t kTiB = kGiB * 1024;

TEST(FormatSizeTest, ZeroBytes) { EXPECT_EQ(FormatSize(0), "0 B"); }

TEST(FormatSizeTest, SingleByte) { EXPECT_EQ(FormatSize(1), "1 B"); }

TEST(FormatSizeTest, JustBelowKibibyteStaysInBytes) { EXPECT_EQ(FormatSize(kKiB - 1), "1023 B"); }

TEST(FormatSizeTest, ExactKibibyte) { EXPECT_EQ(FormatSize(kKiB), "1.0 KiB"); }

TEST(FormatSizeTest, OneAndAHalfKibibytes) { EXPECT_EQ(FormatSize(kKiB + (kKiB / 2)), "1.5 KiB"); }

TEST(FormatSizeTest, UsesBinaryUnitsRatherThanDecimal) {
  // 1000 字节不到 1 KiB，不能按十进制显示成 1.0 KB。
  EXPECT_EQ(FormatSize(1000), "1000 B");
}

TEST(FormatSizeTest, RoundsToOneDecimalPlace) {
  // 1075 / 1024 = 1.0498...，保留一位小数后仍是 1.0。
  EXPECT_EQ(FormatSize(1075), "1.0 KiB");
  // 1076 / 1024 = 1.0507...，进到 1.1。
  EXPECT_EQ(FormatSize(1076), "1.1 KiB");
}

TEST(FormatSizeTest, JustBelowNextUnitCanRoundUpInDisplay) {
  // 进位判断发生在四舍五入之前，1048575 仍落在 KiB，显示时却会变成 1024.0。
  EXPECT_EQ(FormatSize(kMiB - 1), "1024.0 KiB");
  EXPECT_EQ(FormatSize(kGiB - 1), "1024.0 MiB");
  EXPECT_EQ(FormatSize(kTiB - 1), "1024.0 GiB");
}

TEST(FormatSizeTest, ExactMebibyte) { EXPECT_EQ(FormatSize(kMiB), "1.0 MiB"); }

TEST(FormatSizeTest, ExactGibibyte) { EXPECT_EQ(FormatSize(kGiB), "1.0 GiB"); }

TEST(FormatSizeTest, ExactTebibyte) { EXPECT_EQ(FormatSize(kTiB), "1.0 TiB"); }

TEST(FormatSizeTest, OneAndAHalfOfEachUnit) {
  EXPECT_EQ(FormatSize(kMiB + (kMiB / 2)), "1.5 MiB");
  EXPECT_EQ(FormatSize(kGiB + (kGiB / 2)), "1.5 GiB");
  EXPECT_EQ(FormatSize(kTiB + (kTiB / 2)), "1.5 TiB");
}

TEST(FormatSizeTest, DoesNotPromotePastTebibyte) {
  EXPECT_EQ(FormatSize(kTiB * 1024), "1024.0 TiB");
  EXPECT_EQ(FormatSize(kTiB * 1536), "1536.0 TiB");
}

}  // namespace
}  // namespace photon::cli
