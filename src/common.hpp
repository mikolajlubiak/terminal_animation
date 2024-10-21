#pragma once

// std
#include <cstdint>

template <typename T>
T mapValue(T x, T old_min, T old_max, T new_min, T new_max) {
  return new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min);
}

std::uint32_t rgbToInt(std::uint8_t r, std::uint8_t g, std::uint8_t b);

void intToRgb(std::uint32_t colorInt, std::uint8_t &r, std::uint8_t &g,
              std::uint8_t &b);