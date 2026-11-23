/**
 * @file media_type_test.cc
 * @brief media_type.h 的单元测试。
 */

#include "photon/media/media_type.h"

#include "gtest/gtest.h"

namespace photon::media {
namespace {

TEST(DetectByExtensionTest, RecognizesCommonRawFormats) {
  EXPECT_EQ(DetectByExtension("IMG_0001.CR3"), MediaType::kRawImage);
  EXPECT_EQ(DetectByExtension("/照片/2026/DSC_0002.nef"), MediaType::kRawImage);
  EXPECT_EQ(DetectByExtension("a.dng"), MediaType::kRawImage);
}

TEST(DetectByExtensionTest, ExtensionIsCaseInsensitive) {
  EXPECT_EQ(DetectByExtension("a.JPG"), MediaType::kJpegImage);
  EXPECT_EQ(DetectByExtension("a.JpEg"), MediaType::kJpegImage);
  EXPECT_EQ(DetectByExtension("a.HeIc"), MediaType::kHeifImage);
}

TEST(DetectByExtensionTest, RecognizesVideoAndSidecar) {
  EXPECT_EQ(DetectByExtension("clip.mp4"), MediaType::kVideo);
  EXPECT_EQ(DetectByExtension("clip.MOV"), MediaType::kVideo);
  EXPECT_EQ(DetectByExtension("IMG_0001.xmp"), MediaType::kSidecar);
}

TEST(DetectByExtensionTest, UnknownForMissingOrUnrecognizedExtension) {
  EXPECT_EQ(DetectByExtension("README"), MediaType::kUnknown);
  EXPECT_EQ(DetectByExtension("notes.txt"), MediaType::kUnknown);
  EXPECT_EQ(DetectByExtension("trailing."), MediaType::kUnknown);
  EXPECT_EQ(DetectByExtension(".hidden"), MediaType::kUnknown);
}

TEST(MediaTypeCategoryTest, ClassifiesImagesAndVideos) {
  EXPECT_TRUE(IsImage(MediaType::kRawImage));
  EXPECT_TRUE(IsImage(MediaType::kHeifImage));
  EXPECT_FALSE(IsImage(MediaType::kVideo));
  EXPECT_FALSE(IsImage(MediaType::kSidecar));

  EXPECT_TRUE(IsVideo(MediaType::kVideo));
  EXPECT_FALSE(IsVideo(MediaType::kJpegImage));
}

TEST(ToStringTest, EveryTypeHasStableName) {
  EXPECT_EQ(ToString(MediaType::kRawImage), "raw");
  EXPECT_EQ(ToString(MediaType::kJpegImage), "jpeg");
  EXPECT_EQ(ToString(MediaType::kHeifImage), "heif");
  EXPECT_EQ(ToString(MediaType::kPngImage), "png");
  EXPECT_EQ(ToString(MediaType::kVideo), "video");
  EXPECT_EQ(ToString(MediaType::kSidecar), "sidecar");
  EXPECT_EQ(ToString(MediaType::kUnknown), "unknown");
}

}  // namespace
}  // namespace photon::media
