// header
#include "animation_ui.hpp"

// local
#include "slider_with_callback.hpp"

// std
#include <chrono>
#include <memory>

namespace terminal_animation {

AnimationUI::AnimationUI() {
  dir_contents_ = GetDirContents(current_dir_);
  printable_dir_contents_ = FormatDirContents(dir_contents_);
  explorer_window_height_ = static_cast<int>(dir_contents_.size()) + 6;

  screen_.SetCursor(ftxui::Screen::Cursor{
      .x = 0, .y = 0, .shape = ftxui::Screen::Cursor::Hidden});
}

void AnimationUI::Run() {
  thread_canvas_update_ = std::thread(&AnimationUI::UpdateCanvasLoop, this);

  auto main_component = ftxui::Container::Stacked({
      ftxui::Maybe(CreateOptionsWindow() | ftxui::align_right, &show_options_),
      CreateFileExplorer() | ftxui::align_right | ftxui::vcenter,
      ftxui::Maybe(CreateShortcutsWindow(), &show_shortcuts_),
      CreateRenderer(),
  });

  main_component |= CreateEventHandler();
  screen_.Loop(main_component);

  media_to_ascii_->SetContinueRendering(false);
  should_run_.store(false);

  if (thread_render_video_.joinable()) {
    thread_render_video_.join();
  }
  if (thread_canvas_update_.joinable()) {
    thread_canvas_update_.join();
  }
}

ftxui::Component AnimationUI::CreateRenderer() {
  return ftxui::Renderer([this] { return CreateCanvas(); });
}

ftxui::Element AnimationUI::CreateCanvas() {
  auto frame = ftxui::canvas([this](ftxui::Canvas &canvas) {
    std::lock_guard<std::mutex> lock(mutex_canvas_data_);

    for (std::uint32_t i = 0; i < canvas_data_.chars.size(); i++) {
      for (std::uint32_t j = 0; j < canvas_data_.chars[i].size(); j++) {
        const std::uint8_t r = canvas_data_.colors[i][j][0];
        const std::uint8_t g = canvas_data_.colors[i][j][1];
        const std::uint8_t b = canvas_data_.colors[i][j][2];

        canvas.DrawText(i * 2, j * 4,
                        std::string(1, canvas_data_.chars[i][j]),
                        ftxui::Color(r, g, b));
      }
    }
  });
  return frame;
}

ftxui::Component AnimationUI::CreateOptionsWindow() {
  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
          ftxui::Slider(
              ftxui::text("Size") | ftxui::color(ftxui::Color::YellowLight),
              ftxui::SliderWithCallbackOption<std::int32_t>{
                  .callback =
                      [this](std::int32_t size) {
                        if (media_to_ascii_->GetSize() ==
                            static_cast<std::uint32_t>(size)) {
                          return;
                        }

                        media_to_ascii_->SetSize(
                            static_cast<std::uint32_t>(size));

                        if (media_to_ascii_->IsVideo()) {
                          StartVideoRendering();
                        } else {
                          media_to_ascii_->CalculateCharsAndColors(0);
                        }

                        canvas_data_ =
                            media_to_ascii_->GetCharsAndColors(0);
                      },
                  .value = 32,
                  .min = 1,
                  .max = 128,
                  .increment = 1,
                  .color_active = ftxui::Color::YellowLight,
                  .color_inactive = ftxui::Color::YellowLight,
              }),
          ftxui::Renderer([] { return ftxui::separator(); }),
          ftxui::Button("Hide", [this] { show_options_ = false; }) |
              ftxui::center | ftxui::color(ftxui::Color::Yellow),
      }),
      .title = "Options",
      .width = 32,
      .height = 8,
      .render = {},
  });
}

ftxui::Component AnimationUI::CreateFileExplorer() {
  auto on_select = [this] {
    if (std::filesystem::is_directory(dir_contents_[selected_index_])) {
      if (selected_index_ == 0) {
        if (current_dir_.has_parent_path()) {
          current_dir_ = current_dir_.parent_path();
        }
      } else {
        current_dir_ = dir_contents_[selected_index_];
      }

      dir_contents_ = GetDirContents(current_dir_);
      printable_dir_contents_ = FormatDirContents(dir_contents_);
      explorer_window_height_ = static_cast<int>(dir_contents_.size()) + 6;
    } else {
      media_to_ascii_->OpenFile(dir_contents_[selected_index_].string());
      fps_.store(media_to_ascii_->GetFramerate());

      if (media_to_ascii_->IsVideo()) {
        StartVideoRendering();
      } else {
        media_to_ascii_->CalculateCharsAndColors(0);
      }

      canvas_data_ = media_to_ascii_->GetCharsAndColors(0);
    }
  };

  ftxui::MenuOption menu_options;
  menu_options.on_enter = on_select;

  auto explorer_menu =
      Menu(&printable_dir_contents_, &selected_index_, menu_options);

  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
                   explorer_menu | ftxui::flex,
                   ftxui::Renderer([] { return ftxui::separator(); }),
                   ftxui::Button("Open", on_select) | ftxui::center,
               }) |
               ftxui::color(ftxui::Color::Cyan),
      .title = "Explore",
      .width = 30,
      .height = &explorer_window_height_,
      .render = {},
  });
}

void AnimationUI::UpdateCanvasLoop() {
  while (should_run_.load()) {
    if (media_to_ascii_->IsVideo()) {
      std::uint32_t idx = frame_index_.load();
      {
        std::lock_guard<std::mutex> lock(mutex_canvas_data_);
        canvas_data_ = media_to_ascii_->GetCharsAndColors(idx);
      }
      screen_.PostEvent(ftxui::Event::Custom);

      std::uint32_t total = media_to_ascii_->GetTotalFrameCount();
      frame_index_.store((idx + 1) % (total + 1));
    }

    std::uint32_t current_fps = fps_.load();
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000 / std::max(1U, current_fps)));
  }
}

std::vector<std::filesystem::path>
AnimationUI::GetDirContents(const std::filesystem::path &path) const {
  std::vector<std::filesystem::path> contents;

  if (!std::filesystem::exists(path) ||
      !std::filesystem::is_directory(path)) {
    return {};
  }

  contents.push_back("..");
  contents.push_back(GetHomeDirectory());

  std::filesystem::path pictures = BuildHomePath("Pictures");
  if (std::filesystem::exists(pictures) &&
      std::filesystem::is_directory(pictures)) {
    contents.push_back(pictures);
  }

  std::error_code ec;
  for (const auto &entry : std::filesystem::directory_iterator(path, ec)) {
    contents.push_back(entry.path());
  }

  return contents;
}

std::vector<std::string> AnimationUI::FormatDirContents(
    const std::vector<std::filesystem::path> &contents) const {
  std::vector<std::string> formatted;
  formatted.reserve(contents.size());

  for (const auto &entry : contents) {
    if (std::filesystem::is_directory(entry)) {
      formatted.push_back("[DIR] " + entry.filename().string());
    } else {
      formatted.push_back("[FILE] " + entry.filename().string());
    }
  }
  return formatted;
}

std::filesystem::path
AnimationUI::BuildHomePath(const std::string &subdir) const {
  std::filesystem::path home = GetHomeDirectory();
  if (!subdir.empty()) {
    home /= subdir;
  }
  return home;
}

ftxui::Component AnimationUI::CreateShortcutsWindow() {
  return ftxui::Window({
      .inner = ftxui::Container::Vertical({
                   ftxui::Renderer([] {
                     return ftxui::vbox({
                         ftxui::text("q - Quit") | ftxui::flex,
                         ftxui::filler(),
                         ftxui::text("o - Open/hide options") | ftxui::flex,
                         ftxui::filler(),
                         ftxui::text("r - Restart playback") | ftxui::flex,
                         ftxui::separator(),
                     });
                   }),
                   ftxui::Button("Hide",
                                 [this] { show_shortcuts_ = false; }) |
                       ftxui::center,
               }) |
               ftxui::color(ftxui::Color::Violet),
      .title = "Shortcuts",
      .width = 40,
      .height = 9,
      .render = {},
  });
}

ftxui::ComponentDecorator AnimationUI::CreateEventHandler() {
  return ftxui::CatchEvent([this](ftxui::Event event) {
    if (event == ftxui::Event::Character('q')) {
      media_to_ascii_->SetContinueRendering(false);
      should_run_.store(false);
      screen_.ExitLoopClosure()();
      return true;
    }
    if (event == ftxui::Event::Character('r')) {
      frame_index_.store(0);
      return true;
    }
    if (event == ftxui::Event::Character('o')) {
      show_options_ = !show_options_;
      return true;
    }
    return false;
  });
}

void AnimationUI::StartVideoRendering() {
  media_to_ascii_->SetContinueRendering(false);
  if (thread_render_video_.joinable()) {
    thread_render_video_.join();
  }
  media_to_ascii_->SetContinueRendering(true);
  media_to_ascii_->SetCurrentFrameIndex(0);
  thread_render_video_ =
      std::thread(&MediaToAscii::RenderVideo, media_to_ascii_.get());
  frame_index_.store(0);
}

} // namespace terminal_animation