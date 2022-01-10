#include <functional>  // for function
#include <iostream>  // for basic_ostream::operator<<, operator<<, endl, basic_ostream, basic_ostream<>::__ostream_type, cout, ostream
#include <string>    // for string, basic_string, allocator
#include <vector>    // for vector

#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

using namespace ftxui;

namespace helper {

void menu_widget(const std::vector<std::string>& entries,
                 int* selected,
                 ScreenInteractive* screen,
                 const std::function<void()>& ok_callback) {
  MenuOption option{.on_enter = ok_callback};
  auto menu = Menu(&entries, selected, &option);
  auto content = Renderer(menu, [&] { return menu->Render() | center; });

  ButtonOption button_option{.border = false};
  /* clang-format off */
  auto button_ok   = Button("OK", ok_callback, &button_option);
  auto button_quit = Button("Cancel", screen->ExitLoopClosure(), &button_option);
  /* clang-format on */

  auto controls_container = Container::Horizontal({
      button_ok,
      Renderer([] { return filler() | size(WIDTH, GREATER_THAN, 3); }),
      button_quit,
  });

  auto controls = Renderer(controls_container, [&] {
    return controls_container->Render() | hcenter | size(HEIGHT, LESS_THAN, 3) |
           size(WIDTH, GREATER_THAN, 25);
  });

  auto global = Container::Vertical(
      {content, Renderer([] { return separator(); }), controls});

  auto renderer = Renderer(global, [&] {
    return hbox({
        border(vbox({
            global->Render(),
        })),
    });
  });
  screen->Loop(renderer);
}

}  // namespace helper

void execute_interactive() {
  auto screen = ScreenInteractive::TerminalOutput();

  const std::vector<std::string> entries = {
      "bash",
      "zsh",
      "fish",
  };
  int selected = 0;
  auto ok_callback = [&] {
    screen.Uninstall();
    system(entries[selected].c_str());
    screen.Install();

    screen.ExitLoopClosure()();
  };

  helper::menu_widget(entries, &selected, &screen, ok_callback);
}

int main() {
  auto screen = ScreenInteractive::TerminalOutput();

  const std::vector<std::string> entries = {
      "Interactive",
      "Done",
  };
  int selected = 0;
  auto ok_callback = [&] {
    if (selected == 0) {
      execute_interactive();
      return;
    }
    screen.ExitLoopClosure()();
  };

  helper::menu_widget(entries, &selected, &screen, ok_callback);
}

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
