// header
#include "animation_ui.hpp"

// std
#include <future>

namespace terminal_animation {

AnimationUI::AnimationUI() {
  m_Screen.SetCursor(
      ftxui::Screen::Cursor(0, 0, ftxui::Screen::Cursor::Hidden));

  m_Renderer = CreateRenderer();

  auto canvas_updater_handle =
      std::async(std::launch::async, [this] { ForceUpdateCanvas(); });

  MainUI();
}

// Create all needed components and loop
void AnimationUI::MainUI() { m_Screen.Loop(m_Renderer); }

// Create static canvas with the ASCII art
ftxui::Component AnimationUI::CreateRenderer() {
  return ftxui::Renderer([this] { return CreateCanvas(); });
}

// Create static UI game element
ftxui::Element AnimationUI::CreateCanvas() {
  auto frame = ftxui::canvas([this](ftxui::Canvas &canvas) {
    std::uint32_t x = 0;
    std::uint32_t y = 0;
    for (auto c : m_CanvasText) {
      if (c == '\n') {
        y += 4;
        x = 0;
        continue;
      }

      canvas.DrawText(x, y, std::string(1, c));

      x += 2;
    }
  });

  return frame;
}

// Force the update of canvas by submitting an event
void AnimationUI::ForceUpdateCanvas() {
  std::uint32_t fps = m_pVideoToAscii->GetFramerate();

  while (true) {
    m_CanvasText = m_pVideoToAscii->GetNextFrame().str();
    m_Screen.PostEvent(ftxui::Event::Custom); // Send the event

    std::this_thread::sleep_for(std::chrono::milliseconds(
        1000 / fps)); // Sleep for 1s/fps to maintain the fps
  }
}

} // namespace terminal_animation