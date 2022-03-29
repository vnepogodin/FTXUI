#include <functional>  // for function
#include <memory>  // for __shared_ptr_access, __shared_ptr_access<>::element_type, shared_ptr
#include <utility>  // for move

#include "ftxui/component/component.hpp"  // for Component, Make, CatchEvent
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/event.hpp"           // for Event

namespace ftxui {

class CatchEventBase : public ComponentBase {
 public:
  // Constructor.
  explicit CatchEventBase(std::function<bool(const Event&)> on_event)
      : on_event_(std::move(on_event)) {}

  // Component implementation.
  [[nodiscard]] bool OnEvent(const Event& event) noexcept override {
    if (on_event_(event))
      return true;
    return ComponentBase::OnEvent(event);
  }

 protected:
  std::function<bool(const Event&)> on_event_;
};

/// @brief Return a component, using |on_event| to catch events. This function
/// must returns true when the event has been handled, false otherwise.
/// @param child The wrapped component.
/// @param on_event The function drawing the interface.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// auto renderer = Renderer([] {
///   return text("My interface");
/// });
/// auto component = CatchEvent(renderer, [&](Event event) {
///   if (event == Event::Character('q')) {
///     screen.ExitLoopClosure()();
///     return true;
///   }
///   return false;
/// });
/// screen.Loop(component);
/// ```
Component CatchEvent(
    const Component& child,
    const std::function<bool(const Event& event)>& on_event) noexcept {
  auto&& out = Make<CatchEventBase>(on_event);
  out->Add(child);
  return out;
}

/// @brief Decorate a component, using |on_event| to catch events. This function
/// must returns true when the event has been handled, false otherwise.
/// @param on_event The function drawing the interface.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// auto renderer = Renderer([] { return text("Hello world"); });
/// renderer |= CatchEvent([&](Event event) {
///   if (event == Event::Character('q')) {
///     screen.ExitLoopClosure()();
///     return true;
///   }
///   return false;
/// });
/// screen.Loop(renderer);
/// ```
ComponentDecorator CatchEvent(
    const std::function<bool(const Event&)>& on_event) noexcept {
  return [on_event](auto&& child) {
    return CatchEvent(child,
                      [on_event](auto&& event) { return on_event(event); });
  };
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
