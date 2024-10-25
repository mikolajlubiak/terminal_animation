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

  MediaToAscii() = default;

  // Open file
  MediaToAscii(const std::filesystem::path &file) { OpenFile(file); }

  // Release the video
  ~MediaToAscii() { m_VideoCapture.release(); }

  // Open file
  void OpenFile(const std::filesystem::path &file);

  // Loop over video and return ASCII chars and colors
  void RenderNextFrame();

  // Convert a frame to ASCII chars and colors
  void CalculateCharsAndColors();

  CharsAndColors GetCharsAndColors() { return m_CharsAndColors; }

  // Get framerate
  std::uint32_t GetFramerate() const {
    // Return framerate or 1 for images
    return std::max(
        1U, static_cast<std::uint32_t>(m_VideoCapture.get(cv::CAP_PROP_FPS)));
  }

  // Get whether a video or an image is loaded
  bool GetIsVideo() const { return m_IsVideo; }

  bool IsImage(const std::filesystem::path &filename) const {
    return filename.extension() == ".jpg" || filename.extension() == ".jpeg" ||
           filename.extension() == ".png" || filename.extension() == ".bmp";
  }

  // Set blocksize
  void SetHeight(std::uint32_t height) { m_Height = height; }
  void SetWidth(std::uint32_t width) { m_Width = width; }

private: // Attributes
  // Loaded video or image
  bool m_IsVideo = false;

  // Has class loaded any file
  bool m_FileLoaded = false;

  // Blocksize
  std::uint32_t m_Height = 1;
  std::uint32_t m_Width = 1;

  // Vidoe capture
  cv::VideoCapture m_VideoCapture{};

  // Video frame or image
  cv::Mat m_Frame{};

  // Make sure that the program doesn't try to read next frame from
  // the m_VideoCapture and edit the m_VideoCapture, by opening a new video, at
  // the same time.
  std::mutex m_MutexVideo{};

  // Make sure that no two threads try to change the chars and colors data
  std::mutex m_MutexCharsAndColors{};

  // Make sure that no two threads try to call CalculateCharsAndColors at the
  // same time.
  // Make sure that RenderNextFrame won't edit m_Frame while
  // CalculateCharsAndColors is running.
  CharsAndColors m_CharsAndColors{};
};

} // namespace terminal_animation