#pragma once

// local
#include "common.hpp"

// lib
// OpenCV
#include <opencv2/opencv.hpp>

// std
#include <cstdint>
#include <mutex>
#include <vector>

namespace terminal_animation {

class MediaToAscii {
public:
  struct CharsAndColors {
    std::vector<std::vector<std::array<std::uint8_t, 3>>> colors;
    std::vector<std::vector<char>> chars;
  };

  MediaToAscii() = delete;

  // Open file
  MediaToAscii(std::string filename) { OpenFile(filename); }

  // Release the video
  ~MediaToAscii() { m_VideoCapture.release(); }

  // Open file
  void OpenFile(const std::string &filename);

  // Loop over video and return ASCII chars and colors
  CharsAndColors GetCharsAndColorsNextFrame();

  // Convert a frame to ASCII chars and colors
  CharsAndColors GetCharsAndColors();

  // Get framerate
  std::uint32_t GetFramerate() { return m_VideoCapture.get(cv::CAP_PROP_FPS); }

  // Set blocksize
  void SetHeight(std::uint32_t height) { m_Height = height; }
  void SetWidth(std::uint32_t width) { m_Width = width; }

private: // Attributes
  // Loaded video or image
  bool m_IsVideo;

  // Blocksize
  std::uint32_t m_Height = 1;
  std::uint32_t m_Width = 1;

  // Vidoe capture
  cv::VideoCapture m_VideoCapture;

  // Video frame or image
  cv::Mat m_Frame;

  // Mutex for thread safety
  std::mutex m_Mutex;
};

} // namespace terminal_animation