![Animation showcase](docs/showcase.gif "Animation showcase")

## Turn media (video/gif/image) into animated or static ASCII art

# Build
Tested on Linux, but if you manage to get HDF5, VTK and OpenCV working, Windows, MacOS, and really any other OS with a CMake port and C++20 compatible compiler, should work as well.

* Linux (and other Unix systems like MacOS):
    * Install CMake, Git, HDF5, VTK and OpenCV (different commands based on your distribution)
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
* Windows (Native):
    * Step 1: Fight with setting up HDF5, VTK and OpenCV
    * Step 2: Use WSL
    * But seriously, if you manage to get HDF5, VTK and OpenCV working:
* Windows (Visual Studio):
    * `winget install Git.Git`
    * `winget install Kitware.CMake`
    * Make sure you've got the HDF5, VTK and OpenCV working
    * Open Visual Studio and click "Clone a repository"
    * Under "Repository location" type/paste: https://github.com/mikolajlubiak/terminal_animation
    * Click "Clone" and select the folder inside UI
    * Wait for the project to setup, press F5, or run the project from UI
* Windows (Terminal):
    * `winget install Git.Git`
    * `winget install Kitware.CMake`
    * Make sure you've got the HDF5, VTK and OpenCV working
    * `git clone https://github.com/mikolajlubiak/terminal_animation`
    * `cd terminal_animation`
    * `mkdir build`
    * `cd build`
    * `cmake ..`
    * `cmake --build .`
    * `.\terminal_animation`

# Usage
* In the options window you can set the media's (video/gif/image) size
* In the file explorer window you can select the media you want to be turned into ASCII art
