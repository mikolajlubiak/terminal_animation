// header
#include "video_to_ascii.hpp"

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

// Animate the video
std::ostringstream VideoToAscii::GetNextFrame() {
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

// Convert a frame to ASCII
std::ostringstream VideoToAscii::FrameToAscii(cv::Mat frame) {
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

  // Calculate the average luminance for the entire frame
  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      std::uint32_t sum = 0;

      // Calculate the average for each block
      for (std::uint32_t bi = 0; bi < blockSizeX; ++bi) {
        for (std::uint32_t bj = 0; bj < blockSizeY; ++bj) {
          const cv::Vec3b pixel =
              frame.at<cv::Vec3b>(i * blockSizeX + bi, j * blockSizeY + bj);
          const std::uint8_t avg = (pixel[0] + pixel[1] + pixel[2]) / 3;
          sum += avg;
        }
      }

      // Calculate average luminance of the frame region
      frame_average[i][j] = sum / (blockSizeX * blockSizeY);
    }
  }

  // Build the output string
  std::ostringstream output;

  for (std::uint32_t i = 0; i < numBlocksX; i++) {
    for (std::uint32_t j = 0; j < numBlocksY; j++) {
      // Map average to char index
      std::uint32_t density_index =
          mapValue(static_cast<unsigned long>(frame_average[i][j]), 0UL, 255UL,
                   0UL, strlen(density) - 1);

      output << density[density_index];
    }
    output << std::endl;
  }

  return output;
}

} // namespace terminal_animation