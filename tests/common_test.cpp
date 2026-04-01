#include "common.hpp"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace terminal_animation {
namespace {

// --- MapValue tests ---

TEST(MapValueTest, MapsMiddleOfRange) {
  EXPECT_EQ(MapValue(50, 0, 100, 0, 200), 100);
}

TEST(MapValueTest, MapsMinToNewMin) {
  EXPECT_EQ(MapValue(0, 0, 255, 0, 69), 0);
}

TEST(MapValueTest, MapsMaxToNewMax) {
  EXPECT_EQ(MapValue(255, 0, 255, 0, 69), 69);
}

TEST(MapValueTest, HandlesEqualOldRange) {
  EXPECT_EQ(MapValue(5, 5, 5, 10, 20), 10);
}

TEST(MapValueTest, HandlesFloatingPoint) {
  EXPECT_FLOAT_EQ(MapValue(0.5f, 0.0f, 1.0f, 0.0f, 100.0f), 50.0f);
}

TEST(MapValueTest, HandlesNegativeRanges) {
  EXPECT_EQ(MapValue(0, -100, 100, 0, 200), 100);
}

TEST(MapValueTest, DensityArrayFullRange) {
  std::uint32_t max_idx =
      static_cast<std::uint32_t>(kAsciiDensity.size() - 1);

  EXPECT_EQ(MapValue(0U, 0U, 255U, 0U, max_idx), 0U);
  EXPECT_EQ(MapValue(255U, 0U, 255U, 0U, max_idx), max_idx);

  std::uint32_t mid = MapValue(128U, 0U, 255U, 0U, max_idx);
  EXPECT_GT(mid, 0U);
  EXPECT_LT(mid, max_idx);
}

// --- kAsciiDensity tests ---

TEST(AsciiDensityTest, StartsWithDenseChars) {
  EXPECT_EQ(kAsciiDensity.front(), '$');
}

TEST(AsciiDensityTest, EndsWithSpace) {
  EXPECT_EQ(kAsciiDensity.back(), ' ');
}

TEST(AsciiDensityTest, HasReasonableLength) {
  EXPECT_GT(kAsciiDensity.size(), 10u);
  EXPECT_LT(kAsciiDensity.size(), 200u);
}

// --- IsImageExtension tests ---

TEST(IsImageExtensionTest, RecognizesJpg) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("photo.jpg")));
}

TEST(IsImageExtensionTest, RecognizesJpeg) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("photo.jpeg")));
}

TEST(IsImageExtensionTest, RecognizesPng) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("image.png")));
}

TEST(IsImageExtensionTest, RecognizesBmp) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("bitmap.bmp")));
}

TEST(IsImageExtensionTest, RecognizesWebp) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("web.webp")));
}

TEST(IsImageExtensionTest, RecognizesTiff) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("scan.tiff")));
}

TEST(IsImageExtensionTest, RecognizesTif) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("scan.tif")));
}

TEST(IsImageExtensionTest, CaseInsensitive) {
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("PHOTO.JPG")));
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("Image.PNG")));
  EXPECT_TRUE(IsImageExtension(std::filesystem::path("scan.TiFF")));
}

TEST(IsImageExtensionTest, RejectsVideoExtensions) {
  EXPECT_FALSE(IsImageExtension(std::filesystem::path("video.mp4")));
  EXPECT_FALSE(IsImageExtension(std::filesystem::path("movie.avi")));
  EXPECT_FALSE(IsImageExtension(std::filesystem::path("clip.mkv")));
}

TEST(IsImageExtensionTest, RejectsGif) {
  EXPECT_FALSE(IsImageExtension(std::filesystem::path("anim.gif")));
}

TEST(IsImageExtensionTest, RejectsNoExtension) {
  EXPECT_FALSE(IsImageExtension(std::filesystem::path("README")));
}

TEST(IsImageExtensionTest, HandlesPathWithDirectories) {
  EXPECT_TRUE(
      IsImageExtension(std::filesystem::path("/home/user/pics/photo.jpg")));
}

// --- GetHomeDirectory tests ---

TEST(GetHomeDirectoryTest, ReturnsExistingDirectory) {
  std::filesystem::path home = GetHomeDirectory();
  EXPECT_TRUE(std::filesystem::exists(home));
  EXPECT_TRUE(std::filesystem::is_directory(home));
}

// --- ListDirectoryEntries tests ---

TEST(ListDirectoryEntriesTest, ReturnsEmptyForNonexistent) {
  auto entries =
      ListDirectoryEntries(std::filesystem::path("/nonexistent_dir_xyz"));
  EXPECT_TRUE(entries.empty());
}

TEST(ListDirectoryEntriesTest, ListsCurrentDirectory) {
  auto entries = ListDirectoryEntries(std::filesystem::current_path());
  EXPECT_FALSE(entries.empty());
}

TEST(ListDirectoryEntriesTest, ListsTempDirWithCreatedFile) {
  auto tmp = std::filesystem::temp_directory_path() / "ta_test_dir";
  std::filesystem::create_directories(tmp);

  auto cleanup = [&] { std::filesystem::remove_all(tmp); };

  {
    std::ofstream f(tmp / "testfile.txt");
    f << "test";
  }
  auto entries = ListDirectoryEntries(tmp);
  EXPECT_GE(entries.size(), 1u);

  bool found = false;
  for (const auto &e : entries) {
    if (e.filename() == "testfile.txt") {
      found = true;
    }
  }
  EXPECT_TRUE(found);

  cleanup();
}

TEST(ListDirectoryEntriesTest, ReturnsEmptyForFile) {
  auto tmp = std::filesystem::temp_directory_path() / "ta_test_file.txt";
  auto cleanup = [&] { std::filesystem::remove(tmp); };

  {
    std::ofstream f(tmp);
    f << "test";
  }
  auto entries = ListDirectoryEntries(tmp);
  EXPECT_TRUE(entries.empty());

  cleanup();
}

} // namespace
} // namespace terminal_animation
