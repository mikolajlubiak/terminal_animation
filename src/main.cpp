#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

template <typename T>
T mapValue(T x, T old_min, T old_max, T new_min, T new_max) {
  return new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min);
}

int main() {
  const char *density = "Ñ@#W$9876543210?!abc;:+=-,._ ";
  const char *filename = "img2.png";

  int width, height, channels;

  // Load the image
  unsigned char *img_data = stbi_load(filename, &width, &height, &channels, 0);
  if (img_data == nullptr) {
    std::cerr << "Error loading image: " << stbi_failure_reason() << std::endl;
    return 1;
  }

  for (auto i = 0; i < height; i++) {
    for (auto j = 0; j < width; j++) {
      // Calculate the index for the pixel
      int index = (i * width + j) * channels;

      // Get RGB values
      int r = img_data[index];
      int g = img_data[index + 1];
      int b = img_data[index + 2];

      // Calculate average
      int avg = (r + g + b) / 3;

      // Map average to char index
      auto density_index = mapValue(avg, 0, 255, (int)strlen(density), 0);

      // Print RGB values
      char c = density[density_index];

      std::cout << c;
    }
    std::cout << std::endl;
  }

  // Free the image memory
  stbi_image_free(img_data);
  return 0;
}
