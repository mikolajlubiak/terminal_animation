# Architecture

This document describes the internal architecture of **terminal_animation**: how media files are decoded, converted to ASCII art, and rendered in the terminal in real time.

---

## Processing Pipeline

```
┌──────────────┐     ┌──────────────────────────┐     ┌─────────────────┐
│  Media File  │────▶│  Decode  (OpenCV/FFMPEG)  │────▶│  Frame Queue    │
│  (video/gif/ │     │  cv::VideoCapture or      │     │  m_CharsAndColors│
│   image)     │     │  cv::imread               │     │  (std::vector)  │
└──────────────┘     └──────────────────────────┘     └────────┬────────┘
                                                                │
                                                                ▼
┌──────────────────┐     ┌─────────────────────────┐     ┌────────────────┐
│  Terminal Output │◀────│  FTXUI Rendering         │◀────│ ASCII Conversion│
│  (escape codes + │     │  canvas::DrawText +      │     │PixelBlock→char  │
│   characters)    │     │  ftxui::Color(r, g, b)   │     │+ RGB average   │
└──────────────────┘     └─────────────────────────┘     └────────────────┘
```

---

## Multithreading Model

The program uses two dedicated threads on top of the main FTXUI event loop thread:

```
Main thread (FTXUI event loop)
  │
  ├── m_threadRenderVideo      (std::thread)
  │     └── MediaToAscii::RenderVideo()
  │           Continuously decodes frames from cv::VideoCapture and
  │           calls CalculateCharsAndColors() for each frame index.
  │           Writes into m_CharsAndColors[index].
  │           Guarded by: m_MutexVideoCapture, m_MutexFrame,
  │                       m_MutexCharsAndColors
  │
  └── m_threadForceCanvasUpdate  (std::thread)
        └── AnimationUI::ForceUpdateCanvas()
              Reads pre-rendered frames from m_CharsAndColors at the
              correct frame index, copies them to m_CanvasData, then
              posts a Custom event to wake the FTXUI loop.
              Sleeps for (1000 / FPS) ms between frames.
              Guarded by: m_MutexFrameIndex, m_MutexCanvasData
```

### Synchronization Primitives

| Mutex | Protects |
|---|---|
| `m_MutexVideoCapture` | `cv::VideoCapture` operations in `MediaToAscii` |
| `m_MutexFrame` | `cv::Mat m_Frame` in `MediaToAscii` |
| `m_MutexCharsAndColors` | `m_CharsAndColors` vector in `MediaToAscii` |
| `m_MutexCanvasData` | `m_CanvasData` in `AnimationUI` |
| `m_MutexFrameIndex` | `m_FrameIndex` counter in `AnimationUI` |

All mutexes use `std::lock_guard` (RAII) to prevent deadlocks from exceptions.

---

## Media Decoding

`MediaToAscii::OpenFile()` handles two cases:

1. **Image files** (`.jpg`, `.jpeg`, `.png`, `.bmp`): loaded once with `cv::imread()` into `m_Frame`. `m_IsVideo` is set to `false` and a single `CalculateCharsAndColors(0)` call converts it.

2. **Video/GIF files**: opened with `cv::VideoCapture`. If `CAP_PROP_FRAME_COUNT` is 0 (some image formats that FFMPEG handles), the first frame is captured and treated as a static image. Otherwise `m_CharsAndColors` is resized to the total frame count and `m_IsVideo` is set to `true`, triggering `RenderVideo()` on the background thread.

FFMPEG is used transparently by OpenCV via the FFMPEG backend; the `vcpkg.json` manifest explicitly enables the `ffmpeg` feature of the `opencv4` port.

---

## ASCII Conversion

`MediaToAscii::CalculateCharsAndColors()` converts a decoded `cv::Mat` frame into a `CharsAndColors` struct:

```
cv::Mat frame (BGR, full resolution)
        │
        ▼
Divide into blockSizeX × blockSizeY pixel blocks
  │  blockSizeX accounts for FTXUI's 2×4 character cell aspect ratio
  │  blockSizeY = frame.rows / m_Size
        │
        ▼
For each block: average R, G, B over all pixels
        │
        ├──▶ colors[i][j] = { avg_r, avg_g, avg_b }   (stored as uint8_t[3])
        │
        └──▶ luminance = (sum_r + sum_g + sum_b) / (3 * blockPixels)
                │
                ▼
             map_value(luminance, 0, 255, 0, len(density)-1)
                │
                ▼
             chars[i][j] = density[index]
```

The density string (from darkest to lightest):

```
"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
```

---

## TUI Framework (FTXUI)

`AnimationUI` owns the entire interactive interface, built with FTXUI components:

```
ftxui::Container::Stacked
  ├── ftxui::Maybe(GetOptionsWindow(), &m_ShowOptions)   (right-aligned)
  │     └── SliderWithCallback<int32_t> — controls m_Size in MediaToAscii
  ├── GetFileExplorer()                                  (right-aligned, vcenter)
  │     └── ftxui::Menu over m_PrintableCurrentDirContents
  ├── ftxui::Maybe(GetShortcutsWindow(), &m_ShowShortcuts)
  └── CreateRenderer()
        └── ftxui::canvas — draws each ASCII character with ftxui::Color(r,g,b)
```

`ForceUpdateCanvas()` posts `ftxui::Event::Custom` on every frame tick to wake the FTXUI event loop so it re-renders the canvas with the latest data.

### SliderWithCallback

`slider_with_callback.hpp` implements a custom FTXUI slider that invokes a user-supplied `std::function<void(T)>` callback every time the value changes — whether via keyboard, mouse drag, or programmatic set. This component was contributed upstream to FTXUI: [PR #938](https://github.com/ArthurSonzogni/FTXUI/pull/938).

---

## Component Descriptions

| File | Responsibility |
|---|---|
| `main.cpp` | Entry point. Instantiates `AnimationUI` and calls `MainUI()`. |
| `animation_ui.hpp/.cpp` | Top-level UI controller. Owns the FTXUI screen, all windows, both background threads, and the main event loop. |
| `media_to_ascii.hpp/.cpp` | Media decoding and ASCII conversion. Wraps `cv::VideoCapture`, manages frame rendering on a background thread, and exposes `CharsAndColors` data. |
| `slider_with_callback.hpp` | Custom FTXUI slider component with a value-change callback; extends the standard FTXUI slider API. |
| `common.hpp/.cpp` | Shared utilities: `map_value<T>()` for linear range remapping and `get_file_list()` for directory enumeration. |

---

## Performance Considerations

- **Parallel decode and display**: `m_threadRenderVideo` pre-renders all frames into `m_CharsAndColors` as fast as OpenCV/FFMPEG can decode them, while the UI thread reads from the already-converted buffer. This decouples I/O-bound decoding from render-timing.
- **Frame-rate pacing**: `ForceUpdateCanvas()` sleeps for exactly `1000 / FPS` milliseconds between updates, matching the source frame rate without busy-waiting.
- **Aspect ratio correction**: `blockSizeX` uses `m_Size * 2 / aspectRatio` to account for FTXUI's 2×4 pixel character cell geometry, preserving the visual aspect ratio in the terminal.
- **Block averaging**: Instead of mapping every pixel individually, pixels are grouped into rectangular blocks and their average color/luminance is computed. The block size is derived from `m_Size`, allowing the user to trade resolution for performance via the Options slider.
- **Lock granularity**: Each mutex covers only the specific data structure it protects, minimizing contention between the render and decode threads.
