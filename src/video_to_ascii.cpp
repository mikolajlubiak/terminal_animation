// header
#include "video_to_ascii.hpp"

// std
#include <sstream>

namespace terminal_animation {

// Open file
void VideoToAscii::OpenFile(const std::string &filename) {
  // Open the file
  m_VideoCapture.open(filename);

  // If failed to open, note and return
  if (!m_VideoCapture.isOpened()) {
    std::cerr << "Error: Could not open video." << std::endl;
    return;
  }
}

// Loop over video and return ASCII chars and colors
VideoToAscii::CharsAndColors VideoToAscii::GetCharsAndColorsNextFrame() {
  // Read next frame from the video
  m_VideoCapture >> m_Frame;

  // If reached the end of the video, reset the capture
  if (m_Frame.empty()) {
    m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    m_VideoCapture >> m_Frame;
  }

  return GetCharsAndColors();
}

// Convert a frame to ASCII chars and colors
VideoToAscii::CharsAndColors VideoToAscii::GetCharsAndColors() {
  // ASCII density array
  constexpr char density[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
                             "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  // Calculate the number of blocks.
  const std::uint32_t numBlocksX = m_Frame.cols / m_BlockSizeX;
  const std::uint32_t numBlocksY = m_Frame.rows / m_BlockSizeY;

  // Calculate the average luminance for the entire frame and build the output
  // string
  std::vector<std::vector<char>> chars(numBlocksX,
                                       std::vector<char>(numBlocksY));

  std::vector<std::vector<std::array<std::uint8_t, 3>>> colors(
      numBlocksX, std::vector<std::array<std::uint8_t, 3>>(
                      numBlocksY, std::array<std::uint8_t, 3>()));

  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      std::uint32_t sum_r = 0;
      std::uint32_t sum_g = 0;
      std::uint32_t sum_b = 0;

      // Calculate the average for each block
      for (std::uint32_t bi = 0; bi < m_BlockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < m_BlockSizeY; ++bj) {
          const cv::Vec3b pixel = m_Frame.at<cv::Vec3b>(i * m_BlockSizeX + bi,
                                                        j * m_BlockSizeY + bj);

          // OpenCV has BGR color format
          const std::uint8_t b = pixel[0];
          const std::uint8_t g = pixel[1];
          const std::uint8_t r = pixel[2];

          sum_r += r;
          sum_g += g;
          sum_b += b;
        }
      }

      // Calculate R, B and B average color value of the frame region
      colors[i][j][0] = sum_r / (m_BlockSizeX * m_BlockSizeY);
      colors[i][j][1] = sum_g / (m_BlockSizeX * m_BlockSizeY);
      colors[i][j][2] = sum_b / (m_BlockSizeX * m_BlockSizeY);

      // Calculate average luminance of the frame region
      const unsigned long avg =
          (sum_r + sum_g + sum_b) / (3 * m_BlockSizeX * m_BlockSizeY);

      // Map average to char index
      const std::uint32_t density_index =
          mapValue(avg, 0UL, 255UL, 0UL, strlen(density) - 1);

      chars[i][j] = density[density_index];
    }
  }

  return {.colors = colors, .chars = chars};
}

} // namespace terminal_animation