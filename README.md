![Animation showcase](docs/showcase.gif "Animation showcase")

## Turn media (video/gif/image) into colored, animated or static, ASCII art
#### Project development and progress showcase: https://video.infosec.exchange/w/p/vYM6TsSBMGEg1DRHMADXu2

**terminal_animation** is a cross-platform C++ application that converts video files, GIFs, and images into real-time colored ASCII art rendered directly in the terminal. It features a multithreaded frame-processing pipeline — one thread continuously decodes and converts frames via OpenCV/FFMPEG while another drives the FTXUI-based interactive TUI — giving smooth, low-latency playback without blocking the UI.

## Features

- **Real-time video/GIF/image to ASCII conversion** — plays animated media at the source frame rate directly in the terminal
- **Full color support** — each ASCII character is colored using RGB terminal escape codes derived from the original pixel data
- **Multithreaded frame-processing pipeline** — decoding and rendering run on separate threads synchronized with mutexes, keeping the UI responsive
- **Interactive TUI** — FTXUI-powered interface with a live file browser, resizable ASCII output, and keyboard shortcuts
- **Cross-platform** — tested on Linux and Windows (WSL, Visual Studio, and Terminal)
- **Wide media format support** — any format OpenCV/FFMPEG can open: MP4, AVI, MKV, MOV, GIF, JPEG, PNG, BMP, WebP, and more

## Tech Stack

`C++20` `OpenCV` `FFMPEG` `FTXUI` `CMake` `vcpkg` `spdlog`

## Architecture

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for a full description of the processing pipeline, multithreading model, and component breakdown.

For a deep dive into the ASCII rendering subsystem, see [docs/RENDERING_PIPELINE.md](docs/RENDERING_PIPELINE.md).

# Build
Tested on Linux and Windows.

* Linux (and other Unix systems like MacOS):
    * Install necessary packages (different commands based on your distribution)
        * Fedora: 
            * `sudo dnf install cmake git ffmpeg vtk opencv opencv-devel spdlog spdlog-devel`
        * Arch:
            * `sudo pacman -S --needed cmake git ffmpeg vtk opencv spdlog`
        * Ubuntu:
            * `sudo apt install cmake git ffmpeg opencv spdlog`
    * `git clone https://github.com/mikolajlubiak/terminal_animation`
    * `cd terminal_animation`
    * `mkdir build`
    * `cd build`
    * `cmake ..`
    * `cmake --build .`
    * `./terminal_animation`
* Windows (WSL):
    * Setup WSL (Windows Subsystem for Linux)
    * Inside your WSL container do the Linux steps
* Windows (Visual Studio):
    * `winget install Git.Git Kitware.CMake`
    * Open Visual Studio and click "Clone a repository"
    * Under "Repository location" type/paste: https://github.com/mikolajlubiak/terminal_animation
    * Click "Clone" and select the folder inside UI
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * Select preset called "windows", wait for the project to setup (this might take very long)
    * Press F5, or run the project from UI
* Windows (Terminal):
    * `winget install Git.Git Kitware.CMake`
    * `git clone https://github.com/mikolajlubiak/terminal_animation`
    * `cd terminal_animation`
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * `mkdir build`
    * `cd build`
    * `cmake --preset=windows ..` (this might take very long)
    * `cmake --build .`
    * `.\terminal_animation`

# Usage
* In the options window you can set the media's size
* In the file explorer window you can select the media you want to be turned into ASCII art

> [!NOTE]
> # Contribution
> For the purposes of this project, I added functionality to the library I use.
> I wanted the memory board grid to update in real time as the slider value changes.
> There was no way to do that, so I quickly studied the library's source code and added that feature myself.
> I achieved this by making the slider call a callback function each time the slider value change.
> I've made a pull request to the upstream, original library, and it has been merged.
> Pull request: https://github.com/ArthurSonzogni/FTXUI/pull/938
