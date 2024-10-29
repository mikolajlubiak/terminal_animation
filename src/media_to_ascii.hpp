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
  void RenderVideo();

  // Convert a frame to ASCII chars and colors
  void CalculateCharsAndColors(std::uint32_t index);

  CharsAndColors GetCharsAndColors(std::uint32_t index) {
    return m_CharsAndColors[index];
  }

  // Get framerate
  std::uint32_t GetFramerate() const {
    // Return framerate or 1 for images
    return std::max(
        1U, static_cast<std::uint32_t>(m_VideoCapture.get(cv::CAP_PROP_FPS)));
  }

  // Return currently accessed frame index
  std::uint32_t GetCurrentFrameIndex() const {
    return m_VideoCapture.get(cv::CAP_PROP_POS_FRAMES);
  }

  // Return frame count of the currently loaded media
  std::uint32_t GetTotalFrameCount() const {
    return m_VideoCapture.get(cv::CAP_PROP_FRAME_COUNT);
  }

  // Get whether a video or an image is loaded
  bool GetIsVideo() const { return m_IsVideo; }

  bool IsImage(const std::filesystem::path &filename) const {
    return filename.extension() == ".jpg" || filename.extension() == ".jpeg" ||
           filename.extension() == ".png" || filename.extension() == ".bmp";
  }

  // Set blocksize
  void SetSize(std::uint32_t size) { m_Size = size; }

  void SetFrameIndex(std::uint32_t frame_index) {
    m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, frame_index);
  }

private: // Attributes
  // Loaded video or image
  bool m_IsVideo = false;

  // Has class loaded any file
  bool m_FileLoaded = false;

  // Whether to render media
  bool m_RenderMedia = false;

  // Image size
  std::uint32_t m_Size = 1;

  // Vidoe capture
  cv::VideoCapture m_VideoCapture{};

  // Video frame or image
  cv::Mat m_Frame{};

  std::vector<CharsAndColors> m_CharsAndColors{{}};
};

} // namespace terminal_animation