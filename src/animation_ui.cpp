// header
#include "animation_ui.hpp"

// local
#include "slider_with_callback.hpp"

// std
#include <future>
#include <memory>

namespace terminal_animation {

AnimationUI::AnimationUI() {
  m_FPS = m_pVideoToAscii->GetFramerate();

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
      // Options
      ftxui::Maybe(GetOptionsWindow() | ftxui::align_right, &m_ShowOptions),
      // Media explorer
      GetMediaWindow() | ftxui::align_right,

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
ftxui::Component AnimationUI::GetOptionsWindow() {
  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
          // Select ASCII art size
          ftxui::Slider(
              ftxui::text("Size") | ftxui::color(ftxui::Color::YellowLight),
              ftxui::SliderWithCallbackOption<std::int32_t>{
                  .callback =
                      [&](std::int32_t size) {
                        m_pVideoToAscii->SetHeight(size);
                        m_pVideoToAscii->SetWidth(size * 2);

                        m_CanvasData = m_pVideoToAscii->GetCharsAndColors();
                      },
                  .value = 32,
                  .min = 1,
                  .max = 128,
                  .increment = 1,
                  .color_active = ftxui::Color::YellowLight,
                  .color_inactive = ftxui::Color::YellowLight,
              }),

          ftxui::Renderer(
              [] { return ftxui::separator(); }), // Separate select button from
                                                  // options

          // Select/save options
          ftxui::Button("Select", [&] { m_ShowOptions = false; }) |
              ftxui::center | ftxui::color(ftxui::Color::Yellow),
      }),

      .title = "Options",
      .width = 32,
      .height = 8,
  });
}

// Open media window
ftxui::Component AnimationUI::GetMediaWindow() {
  // Open selected media
  auto open_media = [&] {
    m_pVideoToAscii->OpenFile(m_MediaList[m_SelectedMedia]);
    m_CanvasData = m_pVideoToAscii->GetCharsAndColorsNextFrame();
    m_FPS = m_pVideoToAscii->GetFramerate();
  };

  // Menu to select media to open
  ftxui::MenuOption file_explorer_option;
  file_explorer_option.on_enter = open_media;

  auto file_explorer =
      Menu(&m_MediaList, &m_SelectedMedia, file_explorer_option);

  auto media_window =
      ftxui::Window({
          .inner = ftxui::Container::Vertical({
                       file_explorer | ftxui::flex,
                       ftxui::Renderer([] { return ftxui::separator(); }),
                       ftxui::Button("Open", open_media) | ftxui::center,
                   }) |
                   ftxui::color(ftxui::Color::Cyan),

          .title = "Open media",
          .width = 25,
          .height = static_cast<int>(m_MediaList.size()) + 6,
      }) |
      ftxui::vcenter;

  return media_window;
}

// Force the update of canvas by submitting an event
void AnimationUI::ForceUpdateCanvas() {
  while (true) {
    if (m_pVideoToAscii->GetIsVideo()) {
      m_CanvasData = m_pVideoToAscii->GetCharsAndColorsNextFrame();
      m_Screen.PostEvent(ftxui::Event::Custom); // Send the event
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(
        1000 / m_FPS)); // Sleep for 1s/fps to maintain the fps
  }
}

} // namespace terminal_animation