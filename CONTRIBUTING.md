# Contributing

Thank you for your interest in contributing to **terminal_animation**!

---

## Development Environment Setup

### Linux

Install the required packages for your distribution:

**Fedora:**
```bash
sudo dnf install cmake git ffmpeg vtk opencv opencv-devel spdlog spdlog-devel
```

**Arch Linux:**
```bash
sudo pacman -S --needed cmake git ffmpeg vtk opencv spdlog
```

**Ubuntu:**
```bash
sudo apt install cmake git ffmpeg opencv spdlog
```

Then clone and build:

```bash
git clone https://github.com/mikolajlubiak/terminal_animation
cd terminal_animation
mkdir build && cd build
cmake ..
cmake --build .
```

### Windows

1. Install prerequisites:
   ```powershell
   winget install Git.Git Kitware.CMake
   ```
2. Set up vcpkg and add a `VCPKG_ROOT` environment variable:
   [Microsoft vcpkg getting-started guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg)
3. Edit `CMakeUserPresets.json` to point at your vcpkg installation.
4. Build:
   ```powershell
   git clone https://github.com/mikolajlubiak/terminal_animation
   cd terminal_animation
   mkdir build && cd build
   cmake --preset=windows ..
   cmake --build .
   ```

vcpkg reads `vcpkg.json` in the repository root and automatically installs `opencv4` (with the `ffmpeg`, `jpeg`, `png`, and `webp` features) and `spdlog`.

---

## How to Add New Media Format Support

Format support is provided by OpenCV's FFMPEG backend, so most common container formats (MP4, AVI, MKV, MOV, WebM, GIF, etc.) work out of the box.

To add support for an additional format that OpenCV does not handle natively:

1. **Enable the format in vcpkg**: Edit `vcpkg.json` to add the relevant feature to the `opencv4` dependency. Example for HEIF:
   ```json
   {
     "name": "opencv4",
     "default-features": true,
     "features": ["ffmpeg", "jpeg", "png", "webp", "openexr"]
   }
   ```

2. **Register the file extension as an image type** (if it is a static image): In `IsImageExtension()` in `src/common.hpp`, add the new extension to `kImageExtensions`:
   ```cpp
   inline constexpr std::string_view kImageExtensions[] = {
       ".jpg", ".jpeg", ".png", ".bmp", ".webp", ".tiff", ".tif",
       ".heif", // ← add new extension here
   };
   ```
   Extension matching is case-insensitive. If the format is video/animated, no change is needed — `cv::VideoCapture` will handle it automatically.

3. **Test**: Open the new file type through the TUI file browser and verify that it renders correctly.

---

## How to Modify the ASCII Character Mapping

The character density string is defined in `src/common.hpp`:

```cpp
inline constexpr std::string_view kAsciiDensity =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/"
    "\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
```

Characters are ordered from **visually dense** (maps to dark/low-luminance pixels) on the left to **visually sparse** (maps to bright/high-luminance pixels) on the right. The luminance value `[0, 255]` is linearly mapped to an index into this string.

To change the character set:

- **Fewer characters** (coarser gradation): use a shorter string, e.g. `"@#S%?*+;:,. "`.
- **More characters** (finer gradation): add characters between existing ones at the appropriate perceived brightness level.
- **Block characters**: Unicode block characters such as `█▓▒░` can be used on terminals that support UTF-8 — change `char` to a multi-byte character type and update `canvas.DrawText` accordingly.

After editing, rebuild and test with a range of images (bright, dark, and high-contrast) to verify the gradation looks correct.

---

## Code Style Guidelines

The project follows standard modern C++ conventions:

- **Standard**: C++20 (`set(CMAKE_CXX_STANDARD 20)` in `CMakeLists.txt`).
- **Naming**:
  - Classes and structs: `PascalCase` (e.g. `MediaToAscii`, `CharsAndColors`)
  - Member variables: `snake_case_` with trailing underscore (e.g. `frame_index_`, `video_capture_`)
  - Local variables and parameters: `snake_case` (e.g. `block_size_x`)
  - Methods: `PascalCase` (e.g. `OpenFile`, `RenderVideo`)
  - Free functions and constants: `PascalCase` (e.g. `MapValue`, `IsImageExtension`)
  - Constants: `k`-prefixed `PascalCase` (e.g. `kAsciiDensity`, `kImageExtensions`)
- **Namespaces**: Application code lives in the `terminal_animation` namespace.
- **Headers**: Use `#pragma once` instead of include guards.
- **Thread safety**: Use `std::atomic` for simple shared state (booleans, counters). Use `std::lock_guard` (RAII) for mutexes protecting complex data structures; never call `lock()`/`unlock()` manually.
- **Compiler warnings**: The build enables `-Wall -Wextra -pedantic`; new code must compile without warnings.
- **Formatting**: The project ships a `.clang-format` file (Google-based style). Use 2-space indentation.
- **Testing**: Unit tests use Google Test. Run with `cd build && ctest`.
