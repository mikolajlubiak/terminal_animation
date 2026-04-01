// header
#include "media_to_ascii.hpp"

// std
#include <algorithm>
#include <cstdint>
#include <filesystem>

namespace terminal_animation {

void MediaToAscii::OpenFile(const std::filesystem::path &file) {
  should_render_.store(false);

  if (IsImageExtension(file)) {
    std::lock_guard<std::mutex> lock_frame(mutex_frame_);
    frame_ = cv::imread(file.string());
    if (frame_.empty()) {
      logger_->error("[MediaToAscii::OpenFile] Could not open image: {}",
                     file.string());
      return;
    }
    is_video_.store(false);
  } else {
    {
      std::lock_guard<std::mutex> lock_capture(mutex_video_capture_);
      video_capture_.open(file.string());
    }

    if (!video_capture_.isOpened()) {
      logger_->error("[MediaToAscii::OpenFile] Could not open video: {}",
                     file.string());
      return;
    }

    // Some image formats are opened through ffmpeg and report 0 total frames.
    if (GetTotalFrameCount() == 0) {
      std::lock_guard<std::mutex> lock_frame(mutex_frame_);
      video_capture_ >> frame_;
      is_video_.store(false);
    } else {
      std::lock_guard<std::mutex> lock_data(mutex_chars_and_colors_);
      chars_and_colors_.clear();
      chars_and_colors_.resize(GetTotalFrameCount());
      is_video_.store(true);
    }
  }

  should_render_.store(true);
}

void MediaToAscii::RenderVideo() {
  while (GetCurrentFrameIndex() <= GetTotalFrameCount() &&
         should_render_.load()) {
    {
      std::lock_guard<std::mutex> lock_capture(mutex_video_capture_);
      std::lock_guard<std::mutex> lock_frame(mutex_frame_);
      video_capture_ >> frame_;
    }
    // cv::CAP_PROP_POS_FRAMES starts at 1 after the first read.
    CalculateCharsAndColors(
        static_cast<std::uint32_t>(GetCurrentFrameIndex()) - 1);
  }
}

void MediaToAscii::CalculateCharsAndColors(std::uint32_t index) {
  std::lock_guard<std::mutex> lock_data(mutex_chars_and_colors_);
  std::lock_guard<std::mutex> lock_frame(mutex_frame_);

  if (frame_.empty() || frame_.cols == 0 || frame_.rows == 0) {
    return;
  }

  const float aspect_ratio =
      static_cast<float>(frame_.rows) / static_cast<float>(frame_.cols);
  const std::uint32_t current_size = size_.load();

  // Scale X for FTXUI's 2x4 character cell geometry.
  const std::uint32_t block_size_x = std::max(
      1U, static_cast<std::uint32_t>(frame_.cols) /
              static_cast<std::uint32_t>(current_size * 2 / aspect_ratio));
  const std::uint32_t block_size_y =
      std::max(1U, static_cast<std::uint32_t>(frame_.rows) / current_size);

  const std::uint32_t num_blocks_x =
      static_cast<std::uint32_t>(frame_.cols) / block_size_x;
  const std::uint32_t num_blocks_y =
      static_cast<std::uint32_t>(frame_.rows) / block_size_y;

  auto &target = chars_and_colors_[index];
  target.chars.assign(num_blocks_x, std::vector<char>(num_blocks_y));
  target.colors.assign(num_blocks_x,
                       std::vector<std::array<std::uint8_t, 3>>(
                           num_blocks_y, std::array<std::uint8_t, 3>()));

  const std::uint32_t pixels_per_block = block_size_x * block_size_y;
  const auto density_max =
      static_cast<std::uint32_t>(kAsciiDensity.size() - 1);

  for (std::uint32_t i = 0; i < num_blocks_x; i++) {
    for (std::uint32_t j = 0; j < num_blocks_y; j++) {
      std::uint32_t sum_r = 0;
      std::uint32_t sum_g = 0;
      std::uint32_t sum_b = 0;

      for (std::uint32_t bi = 0; bi < block_size_x; ++bi) {
        for (std::uint32_t bj = 0; bj < block_size_y; ++bj) {
          const cv::Vec3b pixel = frame_.at<cv::Vec3b>(
              j * block_size_y + bj, i * block_size_x + bi);
          sum_b += pixel[0];
          sum_g += pixel[1];
          sum_r += pixel[2];
        }
      }

      target.colors[i][j][0] =
          static_cast<std::uint8_t>(sum_r / pixels_per_block);
      target.colors[i][j][1] =
          static_cast<std::uint8_t>(sum_g / pixels_per_block);
      target.colors[i][j][2] =
          static_cast<std::uint8_t>(sum_b / pixels_per_block);

      const std::uint32_t avg_luminance =
          (sum_r + sum_g + sum_b) / (3 * pixels_per_block);

      const std::uint32_t density_index =
          MapValue(avg_luminance, 0U, 255U, 0U, density_max);

      target.chars[i][j] = kAsciiDensity[density_index];
    }
  }
}

MediaToAscii::CharsAndColors
MediaToAscii::GetCharsAndColors(std::uint32_t index) const {
  std::lock_guard<std::mutex> lock_data(mutex_chars_and_colors_);
  if (IsVideo()) {
    if (GetCurrentFrameIndex() < 1) {
      return CharsAndColors{};
    }
    return chars_and_colors_[std::min(GetCurrentFrameIndex() - 1, index)];
  }
  return chars_and_colors_[index];
}

} // namespace terminal_animation