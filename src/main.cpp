#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <thread>
#include <vector>

template <typename T>
T mapValue(T x, T old_min, T old_max, T new_min, T new_max) {
  return new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min);
}

int main() {
  const char *density = "@#W$9876543210?!abc;:+=-,._ ";
  const int blockSize = 6;

  // Open the video file
  cv::VideoCapture cap("gif.gif");
  if (!cap.isOpened()) {
    std::cerr << "Error: Could not open video." << std::endl;
    return -1;
  }

  while (true) {
    // Read a frame from the video
    cv::Mat frame;
    cap >> frame; // Capture the next frame

    if (frame.empty()) {
      // If reached the end of the video, reset the capture
      cap.set(cv::CAP_PROP_POS_FRAMES, 0);
      continue; // Skip to the next iteration to read the first frame again
    }

    // Calculate the number of blocks
    int numBlocksX = frame.cols / blockSize;
    int numBlocksY = frame.rows / blockSize;

    // Use a vector to store the averages
    std::vector<std::vector<int>> arr(numBlocksX, std::vector<int>(numBlocksY));

    // Calculate the average for each block
    for (int i = 0; i < numBlocksX; i++) {
      for (int j = 0; j < numBlocksY; j++) {
        int sum = 0;

        for (int bi = 0; bi < blockSize; ++bi) {
          for (int bj = 0; bj < blockSize; ++bj) {
            cv::Vec3b pixel =
                frame.at<cv::Vec3b>(i * blockSize + bi, j * blockSize + bj);
            auto avg = (pixel[0] + pixel[1] + pixel[2]) / 3;
            sum += avg;
          }
        }

        arr[i][j] = sum / (blockSize * blockSize);
      }
    }

    // Build the output string
    std::ostringstream output;
    for (int i = 0; i < numBlocksX; i++) {
      for (int j = 0; j < numBlocksY; j++) {
        // Map average to char index
        int density_index =
            mapValue(arr[i][j], 0, 255, 0, (int)strlen(density) - 1);
        char c = density[density_index];
        output << c;
      }
      output << std::endl;
    }

    // Clear the terminal and print the output
    std::system("clear");
    std::cout << output.str();

    // Wait for a short period to control the frame rate
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  // Release the video capture object
  cap.release();
  return 0;
}
