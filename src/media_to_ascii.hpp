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
  // Struct with the ASCII art colors and characters to print
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

  // Render the whole video capture to the m_CharsAndColors attribute
  void RenderVideo();

  // Convert a frame to ASCII chars and colors
  void CalculateCharsAndColors(const std::uint32_t index);

  // If the frame is already rendered return the frame else return the newest
  // rendered frame
  CharsAndColors GetCharsAndColors(const std::uint32_t index) const {
    if (GetCurrentFrameIndex() < 1) {
      return CharsAndColors{};
    }

    return m_CharsAndColors[std::min(GetCurrentFrameIndex() - 1, index)];
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
  bool IsVideo() const { return m_IsVideo; }

  // Check the file extension to determine whether its an image
  bool IsImage(const std::filesystem::path &filename) const {
    return filename.extension() == ".jpg" || filename.extension() == ".jpeg" ||
           filename.extension() == ".png" || filename.extension() == ".bmp";
  }

  // Set blocksize
  void SetSize(const std::uint32_t size) { m_Size = size; }

  // Set whether to continue rendering
  void ContinueRendring(const bool should_render) {
    m_ShouldRender = should_render;
  }

private: // Attributes
  // Loaded video or image
  bool m_IsVideo = false;

  // Should you contine rendering?
  bool m_ShouldRender = false;

  // Whether to render media
  bool m_RenderMedia = false;

  // Image size
  std::uint32_t m_Size = 1;

  // Vidoe capture
  cv::VideoCapture m_VideoCapture{};

  // Video frame or image
  cv::Mat m_Frame{};

  // Vector holding the ASCII art information
  std::vector<CharsAndColors> m_CharsAndColors{{}};

  // Make sure that the m_CharsAndColors attribute doesn't get updated in two
  // places at the same time
  std::mutex m_MutexCharsAndColors{};

  // Make sure that the video capture does't get updated in two places
  // at the same time
  std::mutex m_MutexVideoCapture{};

  // Make sure that the frame does't get updated in two places
  // at the same time
  std::mutex m_MutexFrame{};
};

} // namespace terminal_animation