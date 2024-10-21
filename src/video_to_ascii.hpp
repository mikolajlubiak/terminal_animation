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
#include <sstream>
#include <thread>
#include <vector>

namespace terminal_animation {

class VideoToAscii {
public:
  VideoToAscii() {}

  // Open file
  VideoToAscii(std::string filename) { OpenFile(filename); }

  // Release the video
  ~VideoToAscii() { m_VideoCapture.release(); }

  // Open file
  void OpenFile(const std::string &filename);

  // Animate the video
  std::ostringstream GetNextFrame();

  // Convert a frame to ASCII
  std::ostringstream FrameToAscii(cv::Mat frame);

  // Get framerate
  std::uint32_t GetFramerate() { return m_VideoCapture.get(cv::CAP_PROP_FPS); }

private: // Attributes
  cv::VideoCapture m_VideoCapture;
};

} // namespace terminal_animation