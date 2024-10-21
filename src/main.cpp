// local
#include "image_to_ascii.hpp"

int main() {
  VideoToAscii vid_to_ascii = VideoToAscii("gif.gif");

  vid_to_ascii.AnimateVideo();

  return 0;
}
