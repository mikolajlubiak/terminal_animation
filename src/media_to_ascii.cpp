// header
#include "media_to_ascii.hpp"

// std
#include <filesystem>
#include <fstream>

namespace terminal_animation {

// Open file
void MediaToAscii::OpenFile(const std::filesystem::path &file) {
  m_FileLoaded = false;

  // Check if the file is a video or an image
  if (IsImage(file)) {
    // Load image
    m_Frame = cv::imread(file);

    if (m_Frame.empty()) {
      std::ofstream debug_stream("debug_output.txt",
                                 std::ios::app); // Debug output stream

      debug_stream << "[MediaToAscii::OpenFile] Error: Could not open image. "
                   << file << std::endl;

      debug_stream.close();

      return;
    }

    m_IsVideo = false;
  } else {

    // Open the video file
    m_VideoCapture.open(file);

    if (!m_VideoCapture.isOpened()) {
      std::ofstream debug_stream("debug_output.txt",
                                 std::ios::app); // Debug output stream

      debug_stream << "[MediaToAscii::OpenFile] Error: Could not open video. "
                   << file << std::endl;

      debug_stream.close();

      return;
    }

    if (GetTotalFrameCount() == 0) {
      m_VideoCapture >> m_Frame;
      m_IsVideo = false;
    } else {
      m_CharsAndColors.clear();
      m_CharsAndColors.resize(GetTotalFrameCount());

      m_IsVideo = true;
    }
  }

  m_FileLoaded = true;
}

// Loop over video and return ASCII chars and colors
void MediaToAscii::RenderVideo() {
  while (GetCurrentFrameIndex() < GetTotalFrameCount() && m_FileLoaded) {
    m_VideoCapture >> m_Frame;

    CalculateCharsAndColors(GetCurrentFrameIndex() - 1);
  }
}

// Convert a frame to ASCII chars and colors
void MediaToAscii::CalculateCharsAndColors(std::uint32_t index) {
  // ASCII density array
  constexpr char density[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
                             "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  if (m_Frame.empty() || m_Frame.cols == 0 || m_Frame.rows == 0) {
    std::ofstream debug_stream("debug_output.txt",
                               std::ios::app); // Debug output stream

    debug_stream
        << "[MediaToAscii::CalculateCharsAndColors] Warning: Frame is empty."
        << std::endl;

    debug_stream.close();

    return;
  }

  // Calculate the aspect ratio of the original frame
  const float aspectRatio =
      static_cast<float>(m_Frame.cols) / static_cast<float>(m_Frame.rows);

  // Calculate the block size based on desired size
  const std::uint32_t blockSizeX = std::max(
      1U,
      m_Frame.cols /
          (m_Size *
           2)); // Scale the size for X, because FTXUI uses 2x4 size characters

  const std::uint32_t blockSizeY = std::max(
      1U, m_Frame.rows / static_cast<std::uint32_t>(m_Size / aspectRatio));

  // Calculate the number of blocks.
  const std::uint32_t numBlocksX = m_Frame.cols / blockSizeX;
  const std::uint32_t numBlocksY = m_Frame.rows / blockSizeY;

  // Calculate the average colors for the entire frame and build
  // the output string
  m_CharsAndColors[index].chars.clear();
  m_CharsAndColors[index].chars.resize(numBlocksX,
                                       std::vector<char>(numBlocksY));

  m_CharsAndColors[index].colors.clear();
  m_CharsAndColors[index].colors.resize(
      numBlocksX, std::vector<std::array<std::uint8_t, 3>>(
                      numBlocksY, std::array<std::uint8_t, 3>()));

  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      std::uint32_t sum_r = 0;
      std::uint32_t sum_g = 0;
      std::uint32_t sum_b = 0;

      // Calculate the average for each block
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              m_Frame.at<cv::Vec3b>(j * blockSizeY + bj, i * blockSizeX + bi);

          // OpenCV has BGR color format
          const std::uint8_t b = pixel[0];
          const std::uint8_t g = pixel[1];
          const std::uint8_t r = pixel[2];

          sum_r += r;
          sum_g += g;
          sum_b += b;
        }
      }

      // Calculate R, B and B average color value of the frame
      // region
      m_CharsAndColors[index].colors[i][j][0] =
          sum_r / (blockSizeX * blockSizeY);
      m_CharsAndColors[index].colors[i][j][1] =
          sum_g / (blockSizeX * blockSizeY);
      m_CharsAndColors[index].colors[i][j][2] =
          sum_b / (blockSizeX * blockSizeY);

      // Calculate average luminance of the frame region
      const unsigned long avg =
          (sum_r + sum_g + sum_b) / (3 * blockSizeX * blockSizeY);

      // Map average to char index
      const std::uint32_t density_index =
          map_value(avg, 0UL, 255UL, 0UL,
                    static_cast<unsigned long>(strlen(density) - 1));

      m_CharsAndColors[index].chars[i][j] = density[density_index];
    }
  }
}

} // namespace terminal_animation