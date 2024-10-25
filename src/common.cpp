// header
#include "common.hpp"

// std
#include <filesystem>

std::vector<std::filesystem::path>
get_file_list(const std::filesystem::path &directory) {
  std::vector<std::filesystem::path> file_list;
  for (const auto &entry : std::filesystem::directory_iterator(directory)) {
    if (std::filesystem::is_regular_file(entry)) {
      file_list.push_back(entry.path());
    }
  }
  return file_list;
}