#pragma once

// std
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace terminal_animation {

// Linearly maps a value from one range to another.
// Returns new_min when old_min == old_max (avoids division by zero).
template <typename T>
constexpr T MapValue(T x, T old_min, T old_max, T new_min, T new_max) {
  if (old_max == old_min) {
    return new_min;
  }
  return new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min);
}

// ASCII density characters ordered from visually dense (dark) to sparse (bright).
inline constexpr std::string_view kAsciiDensity =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
    "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

// Known image file extensions (lowercase).
inline constexpr std::string_view kImageExtensions[] = {
    ".jpg", ".jpeg", ".png", ".bmp", ".webp", ".tiff", ".tif",
};

// Returns true if the path has a recognized image extension (case-insensitive).
bool IsImageExtension(const std::filesystem::path &path);

// Returns the platform-appropriate home directory, or falls back to cwd.
std::filesystem::path GetHomeDirectory();

// Lists non-hidden entries in a directory. Returns empty on error.
std::vector<std::filesystem::path>
ListDirectoryEntries(const std::filesystem::path &directory);

} // namespace terminal_animation