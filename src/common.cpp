// header
#include "common.hpp"

// std
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace terminal_animation {

bool IsImageExtension(const std::filesystem::path &path) {
  std::string ext = path.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  for (const auto &known : kImageExtensions) {
    if (ext == known) {
      return true;
    }
  }
  return false;
}

std::filesystem::path GetHomeDirectory() {
#ifdef _WIN32
  const char *home = std::getenv("USERPROFILE");
#else
  const char *home = std::getenv("HOME");
#endif
  if (home != nullptr) {
    std::filesystem::path home_path(home);
    if (std::filesystem::exists(home_path) &&
        std::filesystem::is_directory(home_path)) {
      return home_path;
    }
  }
  return std::filesystem::current_path();
}

std::vector<std::filesystem::path>
ListDirectoryEntries(const std::filesystem::path &directory) {
  std::vector<std::filesystem::path> entries;
  if (!std::filesystem::exists(directory) ||
      !std::filesystem::is_directory(directory)) {
    return entries;
  }
  std::error_code ec;
  for (const auto &entry : std::filesystem::directory_iterator(directory, ec)) {
    entries.push_back(entry.path());
  }
  return entries;
}

} // namespace terminal_animation