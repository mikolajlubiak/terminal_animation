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

namespace terminal_animation {

class AnimationUI {
public:
  // Default con
  AnimationUI();

  // Create all needed components and loop
  void MainUI();

private: // Methods
  // Update static UI
  ftxui::Component CreateRenderer() const;

  // Create static canvas with the ASCII art
  ftxui::Element CreateCanvas() const;

  // Window for selecting options
  ftxui::Component GetOptionsWindow();

  // File explorer window
  ftxui::Component GetFileExplorer();

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
  bool m_ShowOptions = true;

  bool m_Beep = false;

  // FPS of the currently animated media
  std::uint32_t m_FPS = 1;

  ftxui::ScreenInteractive m_Screen = ftxui::ScreenInteractive::Fullscreen();

  // Handle video to animated ASCII convertion
  std::unique_ptr<MediaToAscii> m_pVideoToAscii =
      std::make_unique<MediaToAscii>();

  // Text that will be on canvas
  MediaToAscii::CharsAndColors m_CanvasData{};

  // Current directory
  std::filesystem::path m_CurrentDir = std::filesystem::current_path();

  // List with files in the currently explored directory
  std::vector<std::filesystem::path> m_CurrentDirContents =
      GetDirContents(m_CurrentDir);

  // List with files in the currently explored directory
  std::vector<std::string> m_PrintableCurrentDirContents =
      PrintableContents(m_CurrentDirContents);

  // Index of the currently selected file/directory in explorer
  int m_SelectedContentIndex = 0;

  // Explorer window height
  int m_ExplorerWindowHeight =
      static_cast<int>(m_CurrentDirContents.size()) + 6;
};

} // namespace terminal_animation