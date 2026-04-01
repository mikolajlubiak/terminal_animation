#pragma once

// local
#include "common.hpp"

// lib
// OpenCV
#include <opencv2/opencv.hpp>
// spdlog
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

// std
#include <array>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <vector>

namespace terminal_animation {

class MediaToAscii {
public:
  // Per-character RGB color and ASCII character for one frame of output.
  struct CharsAndColors {
    std::vector<std::vector<std::array<std::uint8_t, 3>>> colors;
    std::vector<std::vector<char>> chars;
  };

  MediaToAscii() = default;

  explicit MediaToAscii(const std::filesystem::path &file) { OpenFile(file); }

  ~MediaToAscii() { video_capture_.release(); }

  MediaToAscii(const MediaToAscii &) = delete;
  MediaToAscii &operator=(const MediaToAscii &) = delete;

  // Opens a media file (image or video/GIF).
  void OpenFile(const std::filesystem::path &file);

  // Decodes every frame of the loaded video into chars_and_colors_.
  void RenderVideo();

  // Converts a single frame at the given index to ASCII.
  void CalculateCharsAndColors(std::uint32_t index);

  // Returns the pre-rendered frame data at the given index.
  // For video, returns the latest available frame if index is not yet decoded.
  CharsAndColors GetCharsAndColors(std::uint32_t index) const;

  std::uint32_t GetFramerate() const {
    return std::max(
        1U, static_cast<std::uint32_t>(video_capture_.get(cv::CAP_PROP_FPS)));
  }

  std::uint32_t GetCurrentFrameIndex() const {
    return static_cast<std::uint32_t>(
        video_capture_.get(cv::CAP_PROP_POS_FRAMES));
  }

  std::uint32_t GetTotalFrameCount() const {
    return static_cast<std::uint32_t>(
        video_capture_.get(cv::CAP_PROP_FRAME_COUNT));
  }

  bool IsVideo() const { return is_video_.load(); }

  void SetSize(std::uint32_t size) { size_.store(size); }
  std::uint32_t GetSize() const { return size_.load(); }

  void SetContinueRendering(bool should_render) {
    should_render_.store(should_render);
  }

  void SetCurrentFrameIndex(std::uint32_t index) {
    video_capture_.set(cv::CAP_PROP_POS_FRAMES, index);
  }

private:
  std::atomic<bool> is_video_{false};
  std::atomic<bool> should_render_{false};
  std::atomic<std::uint32_t> size_{1};

  cv::VideoCapture video_capture_;
  cv::Mat frame_;
  std::vector<CharsAndColors> chars_and_colors_{{}};

  mutable std::mutex mutex_chars_and_colors_;
  std::mutex mutex_video_capture_;
  std::mutex mutex_frame_;

  std::shared_ptr<spdlog::logger> logger_ =
      spdlog::basic_logger_mt<spdlog::async_factory>("MediaToAscii",
                                                      "logs/debug.txt");
};

} // namespace terminal_animation