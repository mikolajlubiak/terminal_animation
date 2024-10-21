#pragma once

// local
#include "common.hpp"

// lib
// OpenCV
#include <opencv2/opencv.hpp>

// std
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

namespace terminal_animation {

class VideoToAscii {
public:
  struct CharsAndColors {
    std::vector<std::vector<std::array<std::uint8_t, 3>>> colors;
    std::vector<std::vector<char>> chars;
  };

  VideoToAscii() {}

  // Open file
  VideoToAscii(std::string filename) { OpenFile(filename); }

  // Release the video
  ~VideoToAscii() { m_VideoCapture.release(); }

  // Open file
  void OpenFile(const std::string &filename);

  // Loop over video and return ASCII chars and colors
  CharsAndColors GetNextFrameCharsAndColors();

  // Convert a frame to ASCII chars and colors
  CharsAndColors FrameToCharsAndColors(cv::Mat frame);

  // Loop over video and return ASCII chars
  std::string GetNextFrameAscii();

  // Convert a frame to ASCII chars
  std::string FrameToAscii(cv::Mat frame);

  // Get framerate
  std::uint32_t GetFramerate() { return m_VideoCapture.get(cv::CAP_PROP_FPS); }

private: // Attributes
  cv::VideoCapture m_VideoCapture;
};

} // namespace terminal_animation