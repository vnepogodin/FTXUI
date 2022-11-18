#include <functional>  // for function
#include <memory>  // for make_unique, __shared_ptr_access, __shared_ptr_access<>::element_type, shared_ptr
#include <utility>  // for move

#include <ftxui/component/component.hpp>  // for ComponentDecorator, Maybe, Make
#include <ftxui/component/component_base.hpp>  // for Component, ComponentBase
#include <ftxui/component/event.hpp>           // for Event
#include <ftxui/dom/elements.hpp>              // for Element
#include <ftxui/dom/node.hpp>                  // for Node

namespace ftxui {

Component Maybe(const Component& child, const std::function<bool()>& show) noexcept {
  class Impl : public ComponentBase {
   public:
    explicit Impl(std::function<bool()> show) : show_(std::move(show)) {}

   private:
    Element Render() noexcept override {
      return show_() ? ComponentBase::Render() : std::make_unique<Node>();
    }
    [[nodiscard]] bool Focusable() const noexcept override {
      return show_() && ComponentBase::Focusable();
    }
    bool OnEvent(const Event& event) noexcept override {
      return show_() && ComponentBase::OnEvent(event);
    }

    std::function<bool()> show_;
  };

  auto maybe = Make<Impl>(show);
  maybe->Add(child);
  return maybe;
}

/// @brief Decorate a component. It is shown only when the |show| function
/// returns true.
/// @params show a function returning whether the decoratorated component should
/// be shown.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto component = Renderer([]{ return text("Hello World!"); });
/// auto maybe_component = component | Maybe([&]{ return counter == 42; });
/// ```
ComponentDecorator Maybe(const std::function<bool()>& show) noexcept {
  return [show](auto&& child) mutable {
    return Maybe(child, show);
  };
}

/// @brief Decorate a component |child|. It is shown only when |show| is true.
/// @params child the compoennt to decorate.
/// @params show a boolean. |child| is shown when |show| is true.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto component = Renderer([]{ return text("Hello World!"); });
/// auto maybe_component = Maybe(component, &show);
/// ```
Component Maybe(const Component& child, const bool* show) noexcept {
  return Maybe(child, [show] { return *show; });
}

/// @brief Decorate a component. It is shown only when |show| is true.
/// @params show a boolean. |child| is shown when |show| is true.
/// @ingroup component
///
/// ### Example
///
/// ```cpp
/// auto component = Renderer([]{ return text("Hello World!"); });
/// auto maybe_component = component | Maybe(&show);
/// ```
[[gnu::const]]
ComponentDecorator Maybe(const bool* show) noexcept {
  return [show](auto&& child) { return Maybe(child, show); };
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
