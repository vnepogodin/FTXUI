#include <functional>  // for function
#include <memory>      // for __shared_ptr_access, shared_ptr
#include <utility>     // for move

#include "ftxui/component/captured_mouse.hpp"  // for CapturedMouse
#include "ftxui/component/component.hpp"       // for Make, Renderer
#include "ftxui/component/component_base.hpp"  // for Component, ComponentBase
#include "ftxui/component/event.hpp"           // for Event
#include "ftxui/component/mouse.hpp"           // for Mouse
#include "ftxui/dom/elements.hpp"  // for Element, operator|, reflect
#include "ftxui/screen/box.hpp"    // for Box

namespace ftxui {

/// @brief Return a component, using |render| to render its interface.
/// @param render The function drawing the interface.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// auto renderer = Renderer([] {
///   return text("My interface");
/// });
/// screen.Loop(renderer);
/// ```
Component Renderer(const std::function<Element()>& render) noexcept {
  class Impl : public ComponentBase {
   public:
    explicit Impl(std::function<Element()> render)
        : render_(std::move(render)) {}
    Element Render() noexcept override { return render_(); }
    std::function<Element()> render_;
  };

  return std::make_unique<Impl>(render);
}

/// @brief Return a new Component, similar to |child|, but using |render| as the
/// Component::Render() event.
/// @param child The component to forward events to.
/// @param render The function drawing the interface.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// std::string label = "Click to quit";
/// auto button = Button(&label, screen.ExitLoopClosure());
/// auto renderer = Renderer(button, [&] {
///   return hbox({
///     text("A button:"),
///     button->Render(),
///   });
/// });
/// screen.Loop(renderer);
/// ```
Component Renderer(const Component& child, const std::function<Element()>& render) noexcept {
  Component renderer = Renderer(render);
  renderer->Add(child);
  return renderer;
}

/// @brief Return a focusable component, using |render| to render its interface.
/// @param render The function drawing the interface, taking a boolean telling
/// whether the component is focused or not.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// auto renderer = Renderer([] (bool focused) {
///   if (focused)
///     return text("My interface") | inverted;
///   else
///     return text("My interface");
/// });
/// screen.Loop(renderer);
/// ```
Component Renderer(const std::function<Element(bool)>& render) noexcept {
  class Impl : public ComponentBase {
   public:
    explicit Impl(std::function<Element(bool)> render)
        : render_(std::move(render)) {}

   private:
    Element Render() noexcept override {
      return render_(Focused()) | reflect(box_);
    }
    [[nodiscard]] bool Focusable() const noexcept override { return true; }
    bool OnEvent(const Event& event) noexcept override {
      if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y)) {
        if (!CaptureMouse(event)) {
          return false;
        }

        TakeFocus();
      }

      return false;
    }
    Box box_{};

    std::function<Element(bool)> render_;
  };
  return Make<Impl>(render);
}

/// @brief Decorate a component, by decorating what it renders.
/// @param decorator the function modifying the element it renders.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// auto renderer =
//     Renderer([] { return text("Hello");)
///  | Renderer(bold)
///  | Renderer(inverted);
/// screen.Loop(renderer);
/// ```
ComponentDecorator Renderer(const ElementDecorator& decorator) noexcept {
  return [decorator](auto&& component) {
    return Renderer(component, [component, decorator] {
      return component->Render() | decorator;
    });
  };
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
