![Animation showcase](docs/showcase.gif "Animation showcase")

## Turn media (video/gif/image) into colored, animated or static, ASCII art
#### Project development and progress showcase: https://video.infosec.exchange/w/p/vYM6TsSBMGEg1DRHMADXu2

# Build
Tested on Linux and Windows, but MacOS, and really any other OS with CMake, FFmpeg, VTK, OpenCV ports and C++20 compatible compiler, should work as well.

* Linux (and other Unix systems like MacOS):
    * Install CMake, Git, FFmpeg, VTK and OpenCV (different commands based on your distribution)
        * Fedora: `sudo dnf install cmake git ffmpeg vtk opencv opencv-devel`
        * Arch: `sudo pacman -S --needed cmake git ffmpeg vtk opencv`
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
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * Open Visual Studio and click "Clone a repository"
    * Under "Repository location" type/paste: https://github.com/mikolajlubiak/terminal_animation
    * Click "Clone" and select the folder inside UI
    * Select preset called "windows", wait for the project to setup (this will take very long)
    * Press F5, or run the project from UI
* Windows (Terminal):
    * `winget install Git.Git Kitware.CMake`
    * Setup vcpkg, add `VCPKG_ROOT` environmental variable and edit `CMakeUserPresets.json`: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg
    * `git clone https://github.com/mikolajlubiak/terminal_animation`
    * `cd terminal_animation`
    * `mkdir build`
    * `cd build`
    * `cmake --preset=windows ..`
    * `cmake --build .`  (this will take very long)
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
