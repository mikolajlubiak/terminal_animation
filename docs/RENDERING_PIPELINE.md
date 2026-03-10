# Rendering Pipeline

This document is a deep dive into how **terminal_animation** converts raw pixel data into colored ASCII art displayed in the terminal.

---

## Overview

```
cv::Mat (BGR frame)
      │
      ▼
 Block partitioning
      │
      ▼
 Per-block pixel averaging  ──▶  Average RGB  ──▶  Terminal color
      │
      ▼
 Luminance calculation
      │
      ▼
 Density array index lookup  ──▶  ASCII character
      │
      ▼
 CharsAndColors struct
      │
      ▼
 ftxui::canvas::DrawText(x, y, char, Color(r, g, b))
      │
      ▼
 Terminal output (escape codes + characters)
```

---

## 1. Frame Acquisition

For videos and GIFs, `MediaToAscii::RenderVideo()` calls `cv::VideoCapture::operator>>` in a loop to pull the next decoded frame into `cv::Mat m_Frame`. For static images, `cv::imread()` is used directly. Both paths store the result in the same `m_Frame` member, so the conversion logic is identical regardless of media type.

OpenCV decodes frames in **BGR** (Blue-Green-Red) byte order, not the more common RGB. All channel reads in the codebase account for this:

```cpp
const uint8_t b = pixel[0];   // OpenCV channel 0 = Blue
const uint8_t g = pixel[1];   // OpenCV channel 1 = Green
const uint8_t r = pixel[2];   // OpenCV channel 2 = Red
```

---

## 2. Block Partitioning and Aspect Ratio Correction

The full-resolution frame is divided into a grid of rectangular pixel blocks. Each block maps to exactly one ASCII character cell in the terminal.

```
Terminal character grid (numBlocksX × numBlocksY)
┌───┬───┬───┬───┬───┐
│ $ │ @ │ B │ % │ 8 │  ← row j=0
├───┼───┼───┼───┼───┤
│ & │ W │ M │ # │ * │  ← row j=1
├───┼───┼───┼───┼───┤
│ o │ a │ h │ k │ b │  ← row j=2
└───┴───┴───┴───┴───┘

Each cell represents a blockSizeX × blockSizeY region of pixels.
```

Block sizes are computed as:

```
blockSizeY = frame.rows / m_Size
blockSizeX = frame.cols / (m_Size * 2 / aspectRatio)
```

The `* 2 / aspectRatio` factor in X corrects for FTXUI's character cell geometry. FTXUI's canvas API uses a virtual coordinate system where each cell is **2 units wide and 4 units tall** (`DrawText(i*2, j*4, ...)`). Without correction, the ASCII output would appear horizontally squished. The `2 / aspectRatio` scaling stretches the X blocks to match the terminal's character aspect ratio.

---

## 3. Per-Block Pixel Averaging

For each block `(i, j)`, every pixel within the `blockSizeX × blockSizeY` region is summed:

```cpp
uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
for (uint32_t bi = 0; bi < blockSizeX; ++bi) {
    for (uint32_t bj = 0; bj < blockSizeY; ++bj) {
        cv::Vec3b pixel = m_Frame.at<cv::Vec3b>(
            j * blockSizeY + bj,   // row
            i * blockSizeX + bi    // col
        );
        sum_b += pixel[0];
        sum_g += pixel[1];
        sum_r += pixel[2];
    }
}
uint32_t pixels = blockSizeX * blockSizeY;
uint8_t avg_r = sum_r / pixels;
uint8_t avg_g = sum_g / pixels;
uint8_t avg_b = sum_b / pixels;
```

This simple box-filter average preserves color fidelity at lower resolutions while being fast enough to run in real time.

---

## 4. Luminosity Calculation

Luminance is computed as the arithmetic mean of the three averaged channel values:

```
luminance = (sum_r + sum_g + sum_b) / (3 * blockPixels)
```

This is a perceptual approximation. A more accurate formula would use luminance weights (e.g. `0.2126 R + 0.7152 G + 0.0722 B`), but the simple average produces visually acceptable results for ASCII art and is faster to compute.

The result is a value in `[0, 255]` where 0 is black and 255 is white.

---

## 5. Character Selection

The density string maps the luminance range `[0, 255]` to a sequence of ASCII characters ordered from visually dense (dark) to visually sparse (bright):

```
"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
```

The index into this string is calculated by `map_value()`:

```cpp
uint32_t density_index = map_value(
    avg,                               // input
    0UL, 255UL,                        // input range
    0UL, (unsigned long)(strlen(density) - 1)  // output range
);
chars[i][j] = density[density_index];
```

`map_value<T>` performs a linear interpolation:

```
output = new_min + (x - old_min) * (new_max - new_min) / (old_max - old_min)
```

Dark pixels (luminance near 0) map to `$` or `@` — characters with high ink density. Bright pixels (luminance near 255) map to `.` or ` ` (space) — characters with low ink density.

---

## 6. Color Encoding for Terminal Output

The averaged RGB values are stored directly in `CharsAndColors::colors[i][j]` as a `std::array<uint8_t, 3>`. At render time, `AnimationUI::CreateCanvas()` passes them to FTXUI:

```cpp
canvas.DrawText(
    i * 2, j * 4,                           // FTXUI canvas coordinates
    std::string(1, m_CanvasData.chars[i][j]),
    ftxui::Color(r, g, b)                    // 24-bit RGB color
);
```

FTXUI translates `ftxui::Color(r, g, b)` into the standard **24-bit ANSI escape sequence**:

```
ESC[38;2;<r>;<g>;<b>m
```

This sets the foreground color for the character. No background color is set; the terminal's default background shows through, which creates the characteristic ASCII art look.

---

## 7. Frame Sizing and the `m_Size` Parameter

`m_Size` is the user-controlled resolution knob (range 1–128, default 32). It directly determines the number of ASCII character rows in the output:

```
numBlocksY ≈ m_Size
numBlocksX ≈ m_Size * 2 / aspectRatio
```

Increasing `m_Size` shrinks the pixel blocks, producing a higher-resolution ASCII image at the cost of more computation per frame. Decreasing it produces a coarser, faster render. The FTXUI Options window exposes this as a live slider; changing the value while a video is playing stops the background render thread, resets frame index to 0, and restarts rendering at the new resolution.

---

## 8. Animation Timing

`AnimationUI::ForceUpdateCanvas()` runs on a dedicated thread and drives playback timing:

```cpp
while (m_ShouldRun) {
    if (m_pMediaToAscii->IsVideo()) {
        // Copy pre-rendered frame to canvas data
        m_CanvasData = m_pMediaToAscii->GetCharsAndColors(m_FrameIndex);

        // Wake the FTXUI event loop to redraw
        m_Screen.PostEvent(ftxui::Event::Custom);

        // Advance frame index, wrapping at end
        m_FrameIndex = (m_FrameIndex + 1) % (totalFrames + 1);
    }

    // Pace playback to match source FPS
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_FPS));
}
```

`GetCharsAndColors()` uses a safe fallback: if the background decode thread has not yet reached `index`, it returns the most recently decoded frame instead, preventing blank frames during the initial buffering period.

Playback loops indefinitely. Pressing `r` resets `m_FrameIndex` to 0 to restart from the beginning.
