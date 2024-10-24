// header
#include "animation_ui.hpp"

// local
#include "slider_with_callback.hpp"

// std
#include <future>
#include <memory>

namespace terminal_animation {

AnimationUI::AnimationUI() {
  // Hide cursor
  m_Screen.SetCursor(ftxui::Screen::Cursor{
      .x = 0, .y = 0, .shape = ftxui::Screen::Cursor::Hidden});
}

// Create all needed components and loop
void AnimationUI::MainUI() {
  auto canvas_updater_handle =
      std::async(std::launch::async, [this] { ForceUpdateCanvas(); });

  auto main_component = ftxui::Container::Stacked({
      // Options
      ftxui::Maybe(GetOptionsWindow() | ftxui::align_right, &m_ShowOptions),
      // File explorer
      GetFileExplorer() | ftxui::align_right | ftxui::vcenter,

      // ASCII
      CreateRenderer(),
  });

  m_Screen.Loop(main_component);
}

// Create static canvas with the ASCII art
ftxui::Component AnimationUI::CreateRenderer() const {
  return ftxui::Renderer([this] { return CreateCanvas(); });
}

// Create static canvas with the ASCII art
ftxui::Element AnimationUI::CreateCanvas() const {
  auto frame = ftxui::canvas([this](ftxui::Canvas &canvas) {
    for (std::uint32_t i = 0; i < m_CanvasData.chars.size(); i++) {
      for (std::uint32_t j = 0; j < m_CanvasData.chars[i].size(); j++) {
        const std::uint8_t r = m_CanvasData.colors[i][j][0];
        const std::uint8_t g = m_CanvasData.colors[i][j][1];
        const std::uint8_t b = m_CanvasData.colors[i][j][2];

        canvas.DrawText(i * 2, j * 4, std::string(1, m_CanvasData.chars[i][j]),
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
                        m_pVideoToAscii->SetHeight(size * 2);
                        m_pVideoToAscii->SetWidth(size);

                        m_pVideoToAscii->CalculateCharsAndColors();
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

ftxui::Component AnimationUI::GetFileExplorer() {
  // Go to the directory or load selected file
  auto explorer_on_select = [&] {
    // If selected content is directory
    if (std::filesystem::is_directory(
            m_CurrentDirContents[m_SelectedContentIndex])) {

      // Update current directory
      if (m_SelectedContentIndex ==
          0) { // If selected to go to the parent directory
        if (m_CurrentDir.has_parent_path()) { // If has parent directory
          m_CurrentDir =
              m_CurrentDir.parent_path(); // Go to parent directory/go up
        }
      } else {
        m_CurrentDir =
            m_CurrentDirContents[m_SelectedContentIndex]; // Go to
                                                          // selected
                                                          // directory
      }

      // Update directory contents
      m_CurrentDirContents = GetDirContents(m_CurrentDir);

      // Update printable contents
      m_PrintableCurrentDirContents = PrintableContents(m_CurrentDirContents);

      // Resize the window if contents is larger
      m_ExplorerWindowHeight =
          std::max(m_ExplorerWindowHeight,
                   static_cast<int>(m_CurrentDirContents.size()) + 6);

    } else { // If is file
      // Open the file
      m_pVideoToAscii->OpenFile(m_CurrentDirContents[m_SelectedContentIndex]);

      // Update canvas data
      m_pVideoToAscii->RenderNextFrame();
      m_CanvasData = m_pVideoToAscii->GetCharsAndColors();

      // Update FPS
      m_FPS = m_pVideoToAscii->GetFramerate();
    }
  };

  // Menu to explore the filesystem
  ftxui::MenuOption explorer_menu_options;
  explorer_menu_options.on_enter = explorer_on_select;

  auto explorer_menu = Menu(&m_PrintableCurrentDirContents,
                            &m_SelectedContentIndex, explorer_menu_options);

  // Window for the menu
  auto explorer_window = ftxui::Window({
      .inner = ftxui::Container::Vertical({
                   explorer_menu | ftxui::flex,
                   ftxui::Renderer([] { return ftxui::separator(); }),
                   ftxui::Button("Open", explorer_on_select) | ftxui::center,
               }) |
               ftxui::color(ftxui::Color::Cyan),

      .title = "Explore",
      .width = 30,
      .height = &m_ExplorerWindowHeight,
  });

  return explorer_window;
}

// Force the update of canvas by submitting an event
void AnimationUI::ForceUpdateCanvas() {
  while (true) {
    if (m_pVideoToAscii->GetIsVideo()) {
      m_pVideoToAscii->RenderNextFrame();
      m_CanvasData = m_pVideoToAscii->GetCharsAndColors();
      m_Screen.PostEvent(ftxui::Event::Custom); // Send the event
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(
        1000 / m_FPS)); // Sleep for 1s/fps to maintain the fps
  }
}

// Return path directory contents
std::vector<std::filesystem::path>
AnimationUI::GetDirContents(const std::filesystem::path &path) const {
  std::vector<std::filesystem::path> contents;

  if (!std::filesystem::is_directory(path) || !std::filesystem::exists(path)) {
    return {};
  }

  contents.push_back("..");
  contents.push_back(HomeDirPath(""));
  contents.push_back(HomeDirPath("Pictures"));

  for (const auto &entry : std::filesystem::directory_iterator(path)) {
    contents.push_back(entry.path());
  }

  return contents;
}

// Directory contents in printable form
std::vector<std::string> AnimationUI::PrintableContents(
    const std::vector<std::filesystem::path> &contents) const {
  std::vector<std::string> printable_contents;

  for (const auto &entry : contents) {
    if (std::filesystem::is_directory(entry)) {
      printable_contents.push_back("[DIR] " + entry.filename().string());
    } else {
      printable_contents.push_back("[FILE] " + entry.filename().string());
    }
  }

  return printable_contents;
}

std::filesystem::path
AnimationUI::HomeDirPath(const std::string &dir_name) const {
  std::filesystem::path path;

  // Determine the user's home directory
  const char *homeDir = std::getenv("HOME");
  if (!homeDir) {
    homeDir = std::getenv("USERPROFILE"); // For Windows
  }

  if (homeDir) {
    if (dir_name != "") {
      path = std::filesystem::path(homeDir) / dir_name;
    } else {
      path = std::filesystem::path(homeDir);
    }
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
    }
  }

  return path;
}

} // namespace terminal_animation