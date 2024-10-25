#pragma once

// std
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

template <typename T>
T map_value(T x, T old_min, T old_max, T new_min, T new_max) {
  return new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min);
}

std::vector<std::filesystem::path>
get_file_list(const std::filesystem::path &directory);