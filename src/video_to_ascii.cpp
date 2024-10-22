// header
#include "video_to_ascii.hpp"

// std
#include <filesystem>

namespace terminal_animation {

// Open file
void MediaToAscii::OpenFile(const std::string &filename) {
  std::lock_guard<std::mutex> lock(m_Mutex); // Lock the mutex

  // Check if the file is a video or an image
  if (std::filesystem::path(filename).extension() == ".jpg" ||
      std::filesystem::path(filename).extension() == ".jpeg" ||
      std::filesystem::path(filename).extension() == ".png" ||
      std::filesystem::path(filename).extension() == ".bmp") {
    // Load image
    m_Frame = cv::imread(filename);

    if (m_Frame.empty()) {
      std::cerr << "[MediaToAscii::OpenFile] Error: Could not open image. "
                << filename << std::endl;
      return;
    }

    m_IsVideo = false;
  } else {

    // Open the video file
    m_VideoCapture.open(filename);

    if (!m_VideoCapture.isOpened()) {
      std::cerr << "[MediaToAscii::OpenFile] Error: Could not open video. "
                << filename << std::endl;
      return;
    }

    m_IsVideo = true;
  }
}

// Loop over video and return ASCII chars and colors
MediaToAscii::CharsAndColors MediaToAscii::GetCharsAndColorsNextFrame() {
  std::lock_guard<std::mutex> lock(m_Mutex); // Lock the mutex

  if (m_IsVideo) {
    // Read next frame from the video
    m_VideoCapture >> m_Frame;

    // If reached the end of the video, reset the capture
    if (m_Frame.empty()) {
      m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
      m_VideoCapture >> m_Frame;
    }
  }

  return GetCharsAndColors();
}

// Convert a frame to ASCII chars and colors
MediaToAscii::CharsAndColors MediaToAscii::GetCharsAndColors() {
  // ASCII density array
  constexpr char density[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
                             "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  if (m_Frame.empty() || m_Frame.cols == 0 || m_Frame.rows == 0) {
    std::cerr << "[MediaToAscii::GetCharsAndColors] Error: Frame is empty."
              << std::endl;
    return {.colors = {}, .chars = {}}; // Return empty CharsAndColors
  }

  // Calculate the block size based on user input
  const std::uint32_t blockSizeX = std::max(1U, m_Frame.cols / m_Height);
  const std::uint32_t blockSizeY = std::max(1U, m_Frame.rows / m_Width);

  // Calculate the number of blocks.
  const std::uint32_t numBlocksX = m_Frame.cols / blockSizeX;
  const std::uint32_t numBlocksY = m_Frame.rows / blockSizeY;

  // Calculate the average colors for the entire frame and build
  // the output string
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
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              m_Frame.at<cv::Vec3b>(i * blockSizeX + bi, j * blockSizeY + bj);

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
      colors[i][j][0] = sum_r / (blockSizeX * blockSizeY);
      colors[i][j][1] = sum_g / (blockSizeX * blockSizeY);
      colors[i][j][2] = sum_b / (blockSizeX * blockSizeY);

      // Calculate average luminance of the frame region
      const unsigned long avg =
          (sum_r + sum_g + sum_b) / (3 * blockSizeX * blockSizeY);

      // Map average to char index
      const std::uint32_t density_index =
          mapValue(avg, 0UL, 255UL, 0UL, strlen(density) - 1);

      chars[i][j] = density[density_index];
    }
  }

  return {.colors = colors, .chars = chars};
}

} // namespace terminal_animation