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
  ├── thread_render_video_      (std::thread)
  │     └── MediaToAscii::RenderVideo()
  │           Continuously decodes frames from cv::VideoCapture and
  │           calls CalculateCharsAndColors() for each frame index.
  │           Writes into chars_and_colors_[index].
  │           Guarded by: mutex_video_capture_, mutex_frame_,
  │                       mutex_chars_and_colors_
  │
  └── thread_canvas_update_     (std::thread)
        └── AnimationUI::UpdateCanvasLoop()
              Reads pre-rendered frames from chars_and_colors_ at the
              correct frame index, copies them to canvas_data_, then
              posts a Custom event to wake the FTXUI loop.
              Sleeps for (1000 / FPS) ms between frames.
              Guarded by: mutex_canvas_data_
              Uses std::atomic for: frame_index_, fps_, should_run_
```

### Synchronization Primitives

| Mutex | Protects |
|---|---|
| `mutex_video_capture_` | `cv::VideoCapture` operations in `MediaToAscii` |
| `mutex_frame_` | `cv::Mat frame_` in `MediaToAscii` |
| `mutex_chars_and_colors_` | `chars_and_colors_` vector in `MediaToAscii` |
| `mutex_canvas_data_` | `canvas_data_` in `AnimationUI` |

| Atomic | Protects |
|---|---|
| `should_run_` | Main loop termination flag in `AnimationUI` |
| `fps_` | Current playback frame rate in `AnimationUI` |
| `frame_index_` | Current frame index counter in `AnimationUI` |
| `is_video_` | Whether current media is video/animated in `MediaToAscii` |
| `should_render_` | Whether background rendering should continue in `MediaToAscii` |
| `size_` | ASCII resolution (block size) in `MediaToAscii` |

All mutexes use `std::lock_guard` (RAII) to prevent deadlocks from exceptions.

---

## Media Decoding

`MediaToAscii::OpenFile()` handles two cases:

1. **Image files** (`.jpg`, `.jpeg`, `.png`, `.bmp`, `.webp`, `.tiff`, `.tif`): loaded once with `cv::imread()` into `frame_`. `is_video_` is set to `false` and a single `CalculateCharsAndColors(0)` call converts it.

2. **Video/GIF files**: opened with `cv::VideoCapture`. If `CAP_PROP_FRAME_COUNT` is 0 (some image formats that FFMPEG handles), the first frame is captured and treated as a static image. Otherwise `chars_and_colors_` is resized to the total frame count and `is_video_` is set to `true`, triggering `RenderVideo()` on the background thread.

FFMPEG is used transparently by OpenCV via the FFMPEG backend; the `vcpkg.json` manifest explicitly enables the `ffmpeg` feature of the `opencv4` port.

---

## ASCII Conversion

`MediaToAscii::CalculateCharsAndColors()` converts a decoded `cv::Mat` frame into a `CharsAndColors` struct:

```
cv::Mat frame (BGR, full resolution)
        │
        ▼
Divide into block_size_x × block_size_y pixel blocks
  │  block_size_x accounts for FTXUI's 2×4 character cell aspect ratio
  │  block_size_y = frame.rows / size_
        │
        ▼
For each block: average R, G, B over all pixels
        │
        ├──▶ colors[i][j] = { avg_r, avg_g, avg_b }   (stored as uint8_t[3])
        │
        └──▶ luminance = (sum_r + sum_g + sum_b) / (3 * blockPixels)
                │
                ▼
             MapValue(luminance, 0, 255, 0, len(kAsciiDensity)-1)
                │
                ▼
             chars[i][j] = kAsciiDensity[index]
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
  ├── ftxui::Maybe(CreateOptionsWindow(), &show_options_)   (right-aligned)
  │     └── SliderWithCallback<int32_t> — controls size_ in MediaToAscii
  ├── CreateFileExplorer()                                  (right-aligned, vcenter)
  │     └── ftxui::Menu over printable_dir_contents_
  ├── ftxui::Maybe(CreateShortcutsWindow(), &show_shortcuts_)
  └── CreateRenderer()
        └── ftxui::canvas — draws each ASCII character with ftxui::Color(r,g,b)
```

`UpdateCanvasLoop()` posts `ftxui::Event::Custom` on every frame tick to wake the FTXUI event loop so it re-renders the canvas with the latest data.

### SliderWithCallback

`slider_with_callback.hpp` implements a custom FTXUI slider that invokes a user-supplied `std::function<void(T)>` callback every time the value changes — whether via keyboard, mouse drag, or programmatic set. This component was contributed upstream to FTXUI: [PR #938](https://github.com/ArthurSonzogni/FTXUI/pull/938).

---

## Component Descriptions

| File | Responsibility |
|---|---|
| `main.cpp` | Entry point. Instantiates `AnimationUI` and calls `Run()`. |
| `animation_ui.hpp/.cpp` | Top-level UI controller. Owns the FTXUI screen, all windows, both background threads, and the main event loop. |
| `media_to_ascii.hpp/.cpp` | Media decoding and ASCII conversion. Wraps `cv::VideoCapture`, manages frame rendering on a background thread, and exposes `CharsAndColors` data. |
| `slider_with_callback.hpp` | Custom FTXUI slider component with a value-change callback; extends the standard FTXUI slider API. |
| `common.hpp/.cpp` | Shared utilities: `MapValue<T>()` for linear range remapping, `IsImageExtension()`, `GetHomeDirectory()`, `ListDirectoryEntries()`, and the `kAsciiDensity` constant. |

---

## Performance Considerations

- **Parallel decode and display**: `thread_render_video_` pre-renders all frames into `chars_and_colors_` as fast as OpenCV/FFMPEG can decode them, while the UI thread reads from the already-converted buffer. This decouples I/O-bound decoding from render-timing.
- **Frame-rate pacing**: `UpdateCanvasLoop()` sleeps for exactly `1000 / FPS` milliseconds between updates, matching the source frame rate without busy-waiting.
- **Aspect ratio correction**: `block_size_x` uses `size_ * 2 / aspect_ratio` to account for FTXUI's 2×4 pixel character cell geometry, preserving the visual aspect ratio in the terminal.
- **Block averaging**: Instead of mapping every pixel individually, pixels are grouped into rectangular blocks and their average color/luminance is computed. The block size is derived from `size_`, allowing the user to trade resolution for performance via the Options slider.
- **Lock granularity**: Each mutex covers only the specific data structure it protects, minimizing contention between the render and decode threads. Simple shared counters and flags use `std::atomic` to avoid mutex overhead entirely.
