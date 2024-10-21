#pragma once

// local
#include "video_to_ascii.hpp"

// libs
// FTXUI
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

// std
#include <memory>

namespace terminal_animation {

class AnimationUI {
public:
  AnimationUI();

private: // Methods
  // Create all needed components and loop
  void MainUI();

  // Update static UI
  ftxui::Component CreateRenderer();

  // Create static canvas with the ASCII art
  ftxui::Element CreateCanvas();

  // Force the update of canvas by submitting an event
  void ForceUpdateCanvas();

private: // Attributes
  ftxui::ScreenInteractive m_Screen = ftxui::ScreenInteractive::Fullscreen();

  // Update static UI
  ftxui::Component m_Renderer;

  // Handle video to animated ASCII convertion
  std::unique_ptr<VideoToAscii> m_pVideoToAscii =
      std::make_unique<VideoToAscii>("gif.gif");

  // Text that will be on canvas
  VideoToAscii::CharsAndColors m_CanvasData;
};

} // namespace terminal_animation