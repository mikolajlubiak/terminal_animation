// header
#include "animation_ui.hpp"

// local
#include "common.hpp"
#include "slider_with_callback.hpp"

// std
#include <future>
#include <memory>

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
void AnimationUI::MainUI() {
  auto main_component = ftxui::Container::Stacked({
      // options
      ftxui::Maybe(std::move(OptionsWindow()) | ftxui::align_right,
                   &m_ShowOptions),

      // ASCII
      m_Renderer,
  });

  m_Screen.Loop(main_component);
}

// Create static canvas with the ASCII art
ftxui::Component AnimationUI::CreateRenderer() {
  return ftxui::Renderer([this] { return CreateCanvas(); });
}

// Create static canvas with the ASCII art
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

// Window for selecting options
ftxui::Component AnimationUI::OptionsWindow() {
  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
          // Select block size X
          ftxui::Slider(
              ftxui::text("Height") | ftxui::color(ftxui::Color::YellowLight),
              ftxui::SliderWithCallbackOption<std::int32_t>{
                  .callback =
                      [&](std::int32_t block_size_x) {
                        m_pVideoToAscii->SetBlockSizeX(block_size_x);

                        m_CanvasData = m_pVideoToAscii->GetCharsAndColors();
                      },
                  .value = 4,
                  .min = 1,
                  .max = 8,
                  .increment = 1,
                  .color_active = ftxui::Color::YellowLight,
                  .color_inactive = ftxui::Color::YellowLight,
              }),

          // Select block size Y
          ftxui::Slider(
              ftxui::text("Width") | ftxui::color(ftxui::Color::YellowLight),
              ftxui::SliderWithCallbackOption<std::int32_t>{
                  .callback =
                      [&](std::int32_t block_size_y) {
                        m_pVideoToAscii->SetBlockSizeY(block_size_y);

                        m_CanvasData = m_pVideoToAscii->GetCharsAndColors();
                      },
                  .value = 2,
                  .min = 1,
                  .max = 8,
                  .increment = 1,
                  .color_active = ftxui::Color::YellowLight,
                  .color_inactive = ftxui::Color::YellowLight,
              }),

          ftxui::Renderer(
              [] { return ftxui::separator(); }), // Separate select button from
                                                  // options

          // Select/save options
          ftxui::Button("Select", [&] { m_ShowOptions = false; }) |
              ftxui::center | ftxui::flex | ftxui::color(ftxui::Color::Yellow),
      }),

      .title = "Options",
      .width = 32,
      .height = 8,
  });
}

// Force the update of canvas by submitting an event
void AnimationUI::ForceUpdateCanvas() {
  std::uint32_t fps = m_pVideoToAscii->GetFramerate();

  while (true) {
    m_CanvasData = m_pVideoToAscii->GetCharsAndColorsNextFrame();
    m_Screen.PostEvent(ftxui::Event::Custom); // Send the event

    std::this_thread::sleep_for(std::chrono::milliseconds(
        1000 / fps)); // Sleep for 1s/fps to maintain the fps
  }
}

} // namespace terminal_animation