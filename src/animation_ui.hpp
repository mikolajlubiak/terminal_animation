#pragma once

// local
#include "common.hpp"
#include "media_to_ascii.hpp"

// libs
// FTXUI
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

// std
#include <filesystem>
#include <memory>
#include <thread>

namespace terminal_animation {

class AnimationUI {
public:
  AnimationUI();

  // Create all needed components and loop
  void MainUI();

private: // Methods
  // Update static UI
  ftxui::Component CreateRenderer();

  // Create static canvas with the ASCII art
  ftxui::Element CreateCanvas();

  // Window for selecting options
  ftxui::Component GetOptionsWindow();

  // File explorer window
  ftxui::Component GetFileExplorer();

  // Shortcuts window
  ftxui::Component GetShortcutsWindow();

  // Handle events (shortcuts)
  ftxui::ComponentDecorator HandleEvents();

  // Force the update of canvas by submitting an event
  void ForceUpdateCanvas();

  // Return path directory contents
  std::vector<std::filesystem::path>
  GetDirContents(const std::filesystem::path &path) const;

  // Directory contents in printable form
  std::vector<std::string>
  PrintableContents(const std::vector<std::filesystem::path> &contents) const;

  std::filesystem::path HomeDirPath(const std::string &dir_name) const;

private: // Attributes
  // Show options window
  bool m_ShowOptions = true;

  // Show shortcuts window
  bool m_ShowShortcuts = true;

  // Should the program be running?
  bool m_ShouldRun = true;

  // FPS of the currently animated media
  std::uint32_t m_FPS = 1;

  ftxui::ScreenInteractive m_Screen = ftxui::ScreenInteractive::Fullscreen();

  // Handle video to animated ASCII convertion
  std::unique_ptr<MediaToAscii> m_pMediaToAscii =
      std::make_unique<MediaToAscii>();

  // Text that will be on canvas
  MediaToAscii::CharsAndColors m_CanvasData{};

  // Current directory
  std::filesystem::path m_CurrentDir = std::filesystem::current_path();

  // List with files in the currently explored directory
  std::vector<std::filesystem::path> m_CurrentDirContents;

  // List with files in the currently explored directory
  std::vector<std::string> m_PrintableCurrentDirContents;

  // Index of the currently selected file/directory in explorer
  int m_SelectedContentIndex = 0;

  // Explorer window height
  int m_ExplorerWindowHeight;

  // Index of the frame to render
  std::uint32_t m_FrameIndex = 0;

  // Make sure that no two threads try to change the canvas data
  std::mutex m_MutexCanvasData{};

  // Make sure to not change the m_FrameIndex attribute in two places at the
  // same time
  std::mutex m_MutexFrameIndex{};

  // Thread for updating the canvas
  std::thread m_threadForceCanvasUpdate;

  // Thread for rendering the whole video
  std::thread m_threadRenderVideo;
};

} // namespace terminal_animation