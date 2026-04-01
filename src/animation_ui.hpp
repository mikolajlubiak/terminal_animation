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
// spdlog
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

// std
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace terminal_animation {

class AnimationUI {
public:
  AnimationUI();

  // Runs the main FTXUI event loop and blocks until quit.
  void Run();

private:
  // FTXUI component builders
  ftxui::Component CreateRenderer();
  ftxui::Element CreateCanvas();
  ftxui::Component CreateOptionsWindow();
  ftxui::Component CreateFileExplorer();
  ftxui::Component CreateShortcutsWindow();
  ftxui::ComponentDecorator CreateEventHandler();

  // Background thread entry: posts frame updates to the FTXUI loop.
  void UpdateCanvasLoop();

  // Filesystem helpers
  std::vector<std::filesystem::path>
  GetDirContents(const std::filesystem::path &path) const;

  std::vector<std::string>
  FormatDirContents(const std::vector<std::filesystem::path> &contents) const;

  std::filesystem::path BuildHomePath(const std::string &subdir) const;

  // Starts (or restarts) video rendering on the background thread.
  void StartVideoRendering();

  // UI visibility toggles
  bool show_options_ = true;
  bool show_shortcuts_ = true;

  // Main loop control
  std::atomic<bool> should_run_{true};

  // Playback state
  std::atomic<std::uint32_t> fps_{1};
  std::atomic<std::uint32_t> frame_index_{0};

  ftxui::ScreenInteractive screen_ = ftxui::ScreenInteractive::Fullscreen();

  std::unique_ptr<MediaToAscii> media_to_ascii_ =
      std::make_unique<MediaToAscii>();

  MediaToAscii::CharsAndColors canvas_data_;
  std::mutex mutex_canvas_data_;

  // File explorer state
  std::filesystem::path current_dir_ = std::filesystem::current_path();
  std::vector<std::filesystem::path> dir_contents_;
  std::vector<std::string> printable_dir_contents_;
  int selected_index_ = 0;
  int explorer_window_height_ = 0;

  // Background threads
  std::thread thread_canvas_update_;
  std::thread thread_render_video_;

  std::shared_ptr<spdlog::logger> logger_ =
      spdlog::basic_logger_mt<spdlog::async_factory>("AnimationUI",
                                                      "logs/debug.txt");
};

} // namespace terminal_animation