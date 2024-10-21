// header
#include "animation_ui.hpp"

// local
#include "common.hpp"

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
    for (std::uint32_t i = 0; i < m_CanvasData.chars.size(); i++) {
      for (std::uint32_t j = 0; j < m_CanvasData.chars[i].size(); j++) {
        const std::uint8_t r = m_CanvasData.colors[i][j][0];
        const std::uint8_t g = m_CanvasData.colors[i][j][1];
        const std::uint8_t b = m_CanvasData.colors[i][j][2];

        canvas.DrawText(j * 2, i * 4, std::string(1, m_CanvasData.chars[i][j]),
                        ftxui::Color(r, g, b));
      }
    }
  });

  return frame;
}

// Force the update of canvas by submitting an event
void AnimationUI::ForceUpdateCanvas() {
  std::uint32_t fps = m_pVideoToAscii->GetFramerate();

  while (true) {
    m_CanvasData = m_pVideoToAscii->GetNextFrameCharsAndColors();
    m_Screen.PostEvent(ftxui::Event::Custom); // Send the event

    std::this_thread::sleep_for(std::chrono::milliseconds(
        1000 / fps)); // Sleep for 1s/fps to maintain the fps
  }
}

} // namespace terminal_animation