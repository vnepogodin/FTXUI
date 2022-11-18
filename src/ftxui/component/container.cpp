#include <cstddef>    // for size_t
#include <memory>  // for make_unique, __shared_ptr_access, allocator, shared_ptr, allocator_traits<>::value_type
#include <string_view>  // for std::string_view
#include <utility>      // for move
#include <vector>       // for vector, __alloc_traits<>::value_type
#include <algorithm> // for transform

#include <ftxui/component/component.hpp>  // for Horizontal, Vertical, Tab
#include <ftxui/component/component_base.hpp>  // for Components, Component, ComponentBase
#include <ftxui/component/event.hpp>  // for Event, Event::Tab, Event::TabReverse, Event::ArrowDown, Event::ArrowLeft, Event::ArrowRight, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include <ftxui/component/mouse.hpp>  // for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include <ftxui/dom/elements.hpp>  // for text, Elements, operator|, reflect, Element, hbox, vbox
#include <ftxui/screen/box.hpp>  // for Box

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/min.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace ftxui {

class ContainerBase : public ComponentBase {
 public:
  ContainerBase(const Components& children, int* selector)
      : selector_(selector ? selector : &selected_) {
    for (const auto& child : children) {
      Add(child);
    }
  }

  // Component override.
  bool OnEvent(const Event& event) noexcept override {
    if (event.is_mouse()) {
      return OnMouseEvent(event);
    }

    if (!Focused()) {
      return false;
    }

    if (ActiveChild() && ActiveChild()->OnEvent(event)) {
      return true;
    }

    return EventHandler(event);
  }

  Component ActiveChild() noexcept override {
    if (children_.empty()) {
      return nullptr;
    }

    return children_[*selector_ % children_.size()];
  }

  void SetActiveChild(const ComponentBase* child) noexcept override {
    for (size_t i = 0; i < children_.size(); ++i) {
      if (children_[i].get() == child) {
        *selector_ = static_cast<int32_t>(i);
        return;
      }
    }
  }

 protected:
  // Handlers
  virtual bool EventHandler(const Event& /*unused*/) noexcept { return false; }

  virtual bool OnMouseEvent(const Event& event) noexcept {
    return ComponentBase::OnEvent(event);
  }

  int selected_ = 0;
  int* selector_ = nullptr;

  void MoveSelector(int dir) noexcept {
    for (int i = *selector_ + dir;
         i >= 0 && i < static_cast<int>(children_.size()); i += dir) {
      if (children_[i]->Focusable()) {
        *selector_ = i;
        return;
      }
    }
  }
  void MoveSelectorWrap(int dir) noexcept {
    if (children_.empty()) {
      return;
    }
    for (size_t offset = 1; offset < children_.size(); ++offset) {
      const auto i =
          (*selector_ + offset * dir + children_.size()) % children_.size();
      if (children_[i]->Focusable()) {
        *selector_ = static_cast<int32_t>(i);
        return;
      }
    }
  }
};

class VerticalContainer : public ContainerBase {
 public:
  using ContainerBase::ContainerBase;

  Element Render() noexcept override {
    Elements elements;
    std::transform(children_.begin(), children_.end(), std::back_inserter(elements),
                    [](auto&& it) { return it->Render(); });
    if (elements.empty()) {
      return text("Empty container") | reflect(box_);
    }
    return vbox(elements) | reflect(box_);
  }

  bool EventHandler(const Event& event) noexcept override {
    const int old_selected = *selector_;
    if (event == Event::ArrowUp || event == Event::Character('k')) {
      MoveSelector(-1);
    }
    if (event == Event::ArrowDown || event == Event::Character('j')) {
      MoveSelector(+1);
    }
    if (event == Event::PageUp) {
      for (int i = 0; i < box_.y_max - box_.y_min; ++i) {
        MoveSelector(-1);
      }
    }
    if (event == Event::PageDown) {
      for (int i = 0; i < box_.y_max - box_.y_min; ++i) {
        MoveSelector(1);
      }
    }
    if (event == Event::Home) {
      for (size_t i = 0; i < children_.size(); ++i) {
        MoveSelector(-1);
      }
    }
    if (event == Event::End) {
      for (size_t i = 0; i < children_.size(); ++i) {
        MoveSelector(1);
      }
    }
    if (event == Event::Tab) {
      MoveSelectorWrap(+1);
    }
    if (event == Event::TabReverse) {
      MoveSelectorWrap(-1);
    }

    *selector_ = ranges::max(
        0, ranges::min(static_cast<int>(children_.size()) - 1, *selector_));
    return old_selected != *selector_;
  }

  bool OnMouseEvent(const Event& event) noexcept override {
    if (ContainerBase::OnMouseEvent(event)) {
      return true;
    }

    if (event.mouse().button != Mouse::Button::WheelUp &&
        event.mouse().button != Mouse::Button::WheelDown) {
      return false;
    }

    if (!box_.Contain(event.mouse().x, event.mouse().y)) {
      return false;
    }

    if (event.mouse().button == Mouse::Button::WheelUp) {
      MoveSelector(-1);
    }
    if (event.mouse().button == Mouse::Button::WheelDown) {
      MoveSelector(+1);
    }
    *selector_ = ranges::max(
        0, ranges::min(static_cast<int>(children_.size()) - 1, *selector_));

    return true;
  }

  Box box_;
};

class HorizontalContainer : public ContainerBase {
 public:
  using ContainerBase::ContainerBase;

  Element Render() noexcept override {
    Elements elements;
    std::transform(children_.begin(), children_.end(), std::back_inserter(elements),
                    [](auto&& it) { return it->Render(); });
    if (elements.empty()) {
      return text("Empty container");
    }
    return hbox(elements);
  }

  bool EventHandler(const Event& event) noexcept override {
    int old_selected = *selector_;
    if (event == Event::ArrowLeft || event == Event::Character('h')) {
      MoveSelector(-1);
    }
    if (event == Event::ArrowRight || event == Event::Character('l')) {
      MoveSelector(+1);
    }
    if (event == Event::Tab) {
      MoveSelectorWrap(+1);
    }
    if (event == Event::TabReverse) {
      MoveSelectorWrap(-1);
    }

    *selector_ = ranges::max(
        0, ranges::min(static_cast<int>(children_.size()) - 1, *selector_));
    return old_selected != *selector_;
  }
};

class TabContainer : public ContainerBase {
 public:
  using ContainerBase::ContainerBase;

  Element Render() noexcept override {
    Component active_child = ActiveChild();
    if (active_child) {
      return active_child->Render();
    }
    return text("Empty container");
  }

  [[nodiscard]] bool Focusable() const noexcept override {
    if (children_.empty()) {
      return false;
    }
    return children_[*selector_ % children_.size()]->Focusable();
  }

  bool OnMouseEvent(const Event& event) noexcept override {
    return ActiveChild()->OnEvent(event);
  }
};

namespace Container {

/// @brief A list of components, drawn one by one vertically and navigated
/// vertically using up/down arrow key or 'j'/'k' keys.
/// @param children the list of components.
/// @ingroup component
/// @see ContainerBase
///
/// ### Example
///
/// ```cpp
/// auto container = Container::Vertical({
///   children_1,
///   children_2,
///   children_3,
///   children_4,
/// });
/// ```
Component Vertical(const Components& children) noexcept {
  return Vertical(children, nullptr);
}

/// @brief A list of components, drawn one by one vertically and navigated
/// vertically using up/down arrow key or 'j'/'k' keys.
/// This is useful for implementing a Menu for instance.
/// @param children the list of components.
/// @param selector A reference to the index of the selected children.
/// @ingroup component
/// @see ContainerBase
///
/// ### Example
///
/// ```cpp
/// auto container = Container::Vertical({
///   children_1,
///   children_2,
///   children_3,
///   children_4,
/// });
/// ```
Component Vertical(const Components& children, int* selector) noexcept {
  return std::make_unique<VerticalContainer>(children, selector);
}

/// @brief A list of components, drawn one by one horizontally and navigated
/// horizontally using left/right arrow key or 'h'/'l' keys.
/// @param children the list of components.
/// @ingroup component
/// @see ContainerBase
///
/// ### Example
///
/// ```cpp
/// int selected_children = 2;
/// auto container = Container::Horizontal({
///   children_1,
///   children_2,
///   children_3,
///   children_4,
/// }, &selected_children);
/// ```
Component Horizontal(const Components& children) noexcept {
  return Horizontal(children, nullptr);
}

/// @brief A list of components, drawn one by one horizontally and navigated
/// horizontally using left/right arrow key or 'h'/'l' keys.
/// @param children the list of components.
/// @param selector A reference to the index of the selected children.
/// @ingroup component
/// @see ContainerBase
///
/// ### Example
///
/// ```cpp
/// int selected_children = 2;
/// auto container = Container::Horizontal({
///   children_1,
///   children_2,
///   children_3,
///   children_4,
/// }, selected_children);
/// ```
Component Horizontal(const Components& children, int* selector) noexcept {
  return std::make_unique<HorizontalContainer>(children, selector);
}

/// @brief A list of components, where only one is drawn and interacted with at
/// a time. The |selector| gives the index of the selected component. This is
/// useful to implement tabs.
/// @param children The list of components.
/// @param selector The index of the drawn children.
/// @ingroup component
/// @see ContainerBase
///
/// ### Example
///
/// ```cpp
/// int tab_drawn = 0;
/// auto container = Container::Tab({
///   children_1,
///   children_2,
///   children_3,
///   children_4,
/// }, &tab_drawn);
/// ```
Component Tab(const Components& children, int* selector) noexcept {
  return std::make_unique<TabContainer>(children, selector);
}

}  // namespace Container

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
