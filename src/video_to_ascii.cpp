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
VideoToAscii::CharsAndColors VideoToAscii::GetNextFrameCharsAndColors() {
  // Read a frame from the video
  cv::Mat frame;
  m_VideoCapture >> frame; // Capture the next frame

  // If reached the end of the video, reset the capture
  if (frame.empty()) {
    m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    m_VideoCapture >> frame; // Capture the next frame
  }

  return FrameToCharsAndColors(frame);
}

// Convert a frame to ASCII chars and colors
VideoToAscii::CharsAndColors
VideoToAscii::FrameToCharsAndColors(cv::Mat frame) {
  // ASCII density array
  constexpr char density[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
                             "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  // Calculate the block size based on the frame resolution
  const std::uint32_t blockSizeX = frame.cols / 64;
  const std::uint32_t blockSizeY = frame.rows / 128;

  // Calculate the number of blocks. Always the same - 64 and 128
  const std::uint32_t numBlocksX = frame.cols / blockSizeX; // 64
  const std::uint32_t numBlocksY = frame.rows / blockSizeY; // 128

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
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              frame.at<cv::Vec3b>(i * blockSizeX + bi, j * blockSizeY + bj);

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

// Loop over video and return ASCII chars
std::string VideoToAscii::GetNextFrameAscii() {
  // Read a frame from the video
  cv::Mat frame;
  m_VideoCapture >> frame; // Capture the next frame

  // If reached the end of the video, reset the capture
  if (frame.empty()) {
    m_VideoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
    m_VideoCapture >> frame; // Capture the next frame
  }

  return FrameToAscii(frame);
}

// Convert a frame to ASCII chars
std::string VideoToAscii::FrameToAscii(cv::Mat frame) {
  // ASCII density array
  constexpr char density[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
                             "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

  // Calculate the block size based on the frame resolution
  const std::uint32_t blockSizeX = frame.cols / 64;
  const std::uint32_t blockSizeY = frame.rows / 128;

  // Calculate the number of blocks. Always the same - 64 and 128
  const std::uint32_t numBlocksX = frame.cols / blockSizeX; // 64
  const std::uint32_t numBlocksY = frame.rows / blockSizeY; // 128

  // Use a vector to store the luminance averages
  std::vector<std::vector<std::uint8_t>> frame_average(
      numBlocksX, std::vector<std::uint8_t>(numBlocksY));

  // Calculate the average luminance for the entire frame and build the output
  // string
  std::ostringstream output;

  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      std::uint32_t sum = 0;

      // Calculate the average for each block
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              frame.at<cv::Vec3b>(i * blockSizeX + bi, j * blockSizeY + bj);

          const std::uint8_t r = pixel[0];
          const std::uint8_t g = pixel[1];
          const std::uint8_t b = pixel[2];

          const std::uint8_t avg = (r + g + b) / 3;
          sum += avg;
        }
      }

      // Calculate average luminance of the frame region
      const unsigned long avg = sum / (blockSizeX * blockSizeY);

      // Map average to char index
      const std::uint32_t density_index =
          mapValue(avg, 0UL, 255UL, 0UL, strlen(density) - 1);

      output << density[density_index];
    }
    output << std::endl;
  }

  return output.str();
}

} // namespace terminal_animation