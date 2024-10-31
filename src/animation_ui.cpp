// header
#include "animation_ui.hpp"

// local
#include "slider_with_callback.hpp"

// std
#include <fstream>
#include <future>
#include <memory>

namespace terminal_animation {

AnimationUI::AnimationUI() {
  m_CurrentDirContents = GetDirContents(m_CurrentDir);

  m_PrintableCurrentDirContents = PrintableContents(m_CurrentDirContents);

  m_ExplorerWindowHeight = static_cast<int>(m_CurrentDirContents.size()) + 6;

  // Hide cursor
  m_Screen.SetCursor(ftxui::Screen::Cursor{
      .x = 0, .y = 0, .shape = ftxui::Screen::Cursor::Hidden});
}

// Create all needed components and loop
void AnimationUI::MainUI() {
  auto canvas_updater_handle =
      std::async(std::launch::async, &AnimationUI::ForceUpdateCanvas, this);

  auto main_component = ftxui::Container::Stacked({
      // Options
      ftxui::Maybe(GetOptionsWindow() | ftxui::align_right, &m_ShowOptions),

      // File explorer
      GetFileExplorer() | ftxui::align_right | ftxui::vcenter,

      // Shortcuts
      ftxui::Maybe(GetShortcutsWindow(), &m_ShowShortcuts),

      // ASCII art
      CreateRenderer(),
  });

  main_component |= HandleEvents();

  m_Screen.Loop(main_component);
}

// Create static canvas with the ASCII art
ftxui::Component AnimationUI::CreateRenderer() {
  return ftxui::Renderer([this] { return CreateCanvas(); });
}

// Create static canvas with the ASCII art
ftxui::Element AnimationUI::CreateCanvas() {
  auto frame = ftxui::canvas([this](ftxui::Canvas &canvas) {
    // Make sure that no two threads try to change the canvas data
    std::lock_guard<std::mutex> lock(m_MutexCanvasData);

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

                        m_pMediaToAscii->SetSize(size);

                        if (m_pMediaToAscii->GetIsVideo()) {
                          auto f = std::async(std::launch::async,
                                              &MediaToAscii::RenderVideo,
                                              m_pMediaToAscii.get());
                          m_FrameIndex = 0;
                        } else {
                          m_pMediaToAscii->CalculateCharsAndColors(0);
                        }

                        m_CanvasData = m_pMediaToAscii->GetCharsAndColors(0);
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
          ftxui::Button("Hide", [&] { m_ShowOptions = false; }) |
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

      // Resize the window
      m_ExplorerWindowHeight =
          static_cast<int>(m_CurrentDirContents.size()) + 6;

    } else { // If is file
      // Open the file
      m_pMediaToAscii->OpenFile(
          m_CurrentDirContents[m_SelectedContentIndex].string());

      // Update FPS
      m_FPS = m_pMediaToAscii->GetFramerate();

      // Update canvas data
      if (m_pMediaToAscii->GetIsVideo()) {
        auto f = std::async(std::launch::async, &MediaToAscii::RenderVideo,
                            m_pMediaToAscii.get());
        m_FrameIndex = 0;
      } else {
        m_pMediaToAscii->CalculateCharsAndColors(0);
      }
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
  while (m_ShouldRun) {
    if (m_pMediaToAscii->GetIsVideo()) {      
      {
        // Make sure that no two threads try to change the canvas data
        std::lock_guard<std::mutex> lock(m_MutexCanvasData);

        m_CanvasData = m_pMediaToAscii->GetCharsAndColors(m_FrameIndex);
      }

      m_Screen.PostEvent(ftxui::Event::Custom); // Send the event

      m_FrameIndex = (m_FrameIndex + 1) % m_pMediaToAscii->GetTotalFrameCount();
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
#ifdef linux
  const char *homeDir = std::getenv("HOME");
#elif _WIN32
  const char* homeDir = std::getenv("USERPROFILE"); // For Windows
#else
#error shucks!
#endif


  if (homeDir) {
    if (dir_name != "") {
      path = std::filesystem::path(homeDir) / dir_name;
    } else {
      path = std::filesystem::path(homeDir);
    }
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
      return path;
    } else {
      std::ofstream debug_stream("debug_output.txt",
                                 std::ios::app); // Debug output stream

      debug_stream
          << "[AnimationUI::HomeDirPath] Error: Given directory doesn't exist. "
          << path << std::endl;

      debug_stream.close();
    }
  }

  // If there directory doesn't exist, fallback to current working directory.
  return std::filesystem::current_path();
}

// Shortcuts window
ftxui::Component AnimationUI::GetShortcutsWindow() {
  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
                   // Shortcuts
                   ftxui::Renderer([] {
                     return ftxui::vbox({
                         ftxui::text("q - Quit") | ftxui::flex,
                         ftxui::filler(),
                         ftxui::text("o - Open/hide options") | ftxui::flex,
                         ftxui::filler(),
                         ftxui::text("r - Start video/gif from the beginning") |
                             ftxui::flex,
                         ftxui::separator(),
                     });
                   }),
                   // Hide window
                   ftxui::Button("Hide", [&] { m_ShowShortcuts = false; }) |
                       ftxui::center,
               }) |
               ftxui::color(ftxui::Color::Violet),

      .title = "Shortcuts",
      .width = 40,
      .height = 9,
  });
}

// Handle events (arrows and enter)
ftxui::ComponentDecorator AnimationUI::HandleEvents() {
  return ftxui::CatchEvent([this](ftxui::Event event) {
    if (event == ftxui::Event::Character('q')) {
      m_ShouldRun = false;
      m_Screen.ExitLoopClosure()();
      return true;
    } else if (event == ftxui::Event::Character('r')) {
      m_FrameIndex = 0;
      return true;
    } else if (event == ftxui::Event::Character('o')) {
      m_ShowOptions = !m_ShowOptions;
      return true;
    }

    return false;
  });
}

} // namespace terminal_animation