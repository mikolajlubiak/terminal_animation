// header
#include "common.hpp"

// std
#include <filesystem>

std::uint32_t rgbToInt(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
  return (static_cast<std::uint32_t>(r) << 16) |
         (static_cast<std::uint32_t>(g) << 8) | static_cast<std::uint32_t>(b);
}

void intToRgb(std::uint32_t colorInt, std::uint8_t &r, std::uint8_t &g,
              std::uint8_t &b) {
  r = (colorInt >> 16) & 0xFF; // Extract the red component
  g = (colorInt >> 8) & 0xFF;  // Extract the green component
  b = colorInt & 0xFF;         // Extract the blue component
}

std::vector<std::string> get_file_list(const std::string &directory) {
  std::vector<std::string> file_list;
  for (const auto &entry : std::filesystem::directory_iterator(directory)) {
    if (std::filesystem::is_regular_file(entry)) {
      file_list.push_back(entry.path().string());
    }
  }
  return file_list;
}