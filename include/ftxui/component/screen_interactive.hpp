#ifndef FTXUI_COMPONENT_SCREEN_INTERACTIVE_HPP
#define FTXUI_COMPONENT_SCREEN_INTERACTIVE_HPP

#include <atomic>                        // for atomic
#include <ftxui/component/receiver.hpp>  // for Receiver, Sender
#include <functional>                    // for function
#include <memory>                        // for shared_ptr
#include <string>                        // for string
#include <thread>                        // for thread
#include <variant>                       // for variant

#include <ftxui/component/animation.hpp>       // for TimePoint
#include <ftxui/component/captured_mouse.hpp>  // for CapturedMouse
#include <ftxui/component/event.hpp>           // for Event
#include <ftxui/component/task.hpp>            // for Closure, Task
#include <ftxui/screen/screen.hpp>             // for Screen

namespace ftxui {
class ComponentBase;
struct Event;

using Component = std::shared_ptr<ComponentBase>;
class [[maybe_unused]] ScreenInteractivePrivate;

class ScreenInteractive : public Screen {
 public:
  // Constructors:
  static ScreenInteractive FixedSize(int dimx, int dimy) noexcept;
  static ScreenInteractive Fullscreen() noexcept;
  static ScreenInteractive FitComponent() noexcept;
  static ScreenInteractive TerminalOutput() noexcept;

  // Return the currently active screen, nullptr if none.
  static ScreenInteractive* Active() noexcept;

  void Loop(const Component&) noexcept;
  Closure ExitLoopClosure() noexcept;

  void Post(const Task& task) noexcept;
  void PostEvent(const Event& event) noexcept;
  void RequestAnimationFrame() noexcept;

  CapturedMouse CaptureMouse() noexcept;

  // Decorate a function. The outputted one will execute similarly to the
  // inputted one, but with the currently active screen terminal hooks
  // temporarily uninstalled.
  Closure WithRestoredIO(const Closure&) noexcept;

  void Resume() noexcept;
  void Suspend() noexcept;

 private:
  void Install() noexcept;
  void Uninstall() noexcept;

  void Main(const Component& component) noexcept;

  void Draw(const Component& component) noexcept;
  void SigStop() noexcept;

  ScreenInteractive* suspended_screen_ = nullptr;
  enum class Dimension {
    FitComponent,
    Fixed,
    Fullscreen,
    TerminalOutput,
  };
  Dimension dimension_ = Dimension::Fixed;
  bool use_alternative_screen_ = false;
  ScreenInteractive(int dimx,
                    int dimy,
                    Dimension dimension,
                    bool use_alternative_screen) noexcept;

  Sender<Task> task_sender_;
  Receiver<Task> task_receiver_;

  std::string set_cursor_position;
  std::string reset_cursor_position;

  std::atomic<bool> quit_ = false;
  std::thread event_listener_;
  std::thread animation_listener_;
  bool animation_requested_ = false;
  animation::TimePoint previous_animation_time;

  int cursor_x_ = 1;
  int cursor_y_ = 1;

  bool suspended_ = false;
  bool mouse_captured = false;
  bool previous_frame_resized_ = false;

 public:
  class [[maybe_unused]] Private {
   public:
    [[maybe_unused]] static void SigStop(ScreenInteractive& s) { return s.SigStop(); }
  };
  friend Private;
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_SCREEN_INTERACTIVE_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
