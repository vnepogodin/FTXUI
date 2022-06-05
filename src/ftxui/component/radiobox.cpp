#include <algorithm>   // for max
#include <functional>  // for function
#include <memory>      // for shared_ptr, allocator_traits<>::value_type
#include <ranges>
#include <string>   // for string
#include <utility>  // for move
#include <vector>   // for vector

#include <ftxui/component/captured_mouse.hpp>     // for CapturedMouse
#include <ftxui/component/component.hpp>          // for Make, Radiobox
#include <ftxui/component/component_base.hpp>     // for ComponentBase
#include <ftxui/component/component_options.hpp>  // for RadioboxOption
#include <ftxui/component/event.hpp>  // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp, Event::Return, Event::Tab, Event::TabReverse
#include <ftxui/component/mouse.hpp>  // for Mouse, Mouse::WheelDown, Mouse::WheelUp, Mouse::Left, Mouse::Released
#include <ftxui/component/screen_interactive.hpp>  // for Component
#include <ftxui/dom/elements.hpp>  // for operator|, reflect, text, Element, hbox, vbox, Elements, focus, nothing, select
#include <ftxui/screen/box.hpp>    // for Box
#include <ftxui/screen/util.hpp>   // for clamp
#include <ftxui/util/ref.hpp>      // for Ref, ConstStringListRef

namespace ranges = std::ranges;

namespace ftxui {

namespace {
/// @brief A list of selectable element. One and only one can be selected at
/// the same time.
/// @ingroup component
class RadioboxBase : public ComponentBase {
 public:
  RadioboxBase(ConstStringListRef entries,
               int* selected,
               Ref<RadioboxOption> option)
      : selected_(selected), option_(std::move(option)), entries_(entries) {
    hovered_ = *selected_;
  }

 private:
  Element Render() noexcept override {
    Clamp();
    Elements elements;

    const bool is_menu_focused = Focused();
    for (int i = 0; i < size(); ++i) {
      const bool is_focused = (focused_entry() == i) && is_menu_focused;
      const bool is_selected = (hovered_ == i);
      const auto focus_management = !is_selected      ? nothing
                                    : is_menu_focused ? focus
                                                      : select;
      const auto state = EntryState{
          *selected_ == i,
          is_selected,
          is_focused,
          entries_[i],
      };
      const auto element =
          (option_->transform ? option_->transform
                              : RadioboxOption::Simple().transform)(state);

      elements.push_back(element | focus_management | reflect(boxes_[i]));
    }
    return vbox(std::move(elements)) | reflect(box_);
  }

  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  bool OnEvent(const Event& event) noexcept override {
    Clamp();
    if (!CaptureMouse(event)) {
      return false;
    }

    if (event.is_mouse()) {
      return OnMouseEvent(event);
    }

    if (Focused()) {
      const int old_hovered = hovered_;
      if (event == Event::ArrowUp || event == Event::Character('k')) {
        (hovered_)--;
      }
      if (event == Event::ArrowDown || event == Event::Character('j')) {
        (hovered_)++;
      }
      if (event == Event::PageUp) {
        (hovered_) -= box_.y_max - box_.y_min;
      }
      if (event == Event::PageDown) {
        (hovered_) += box_.y_max - box_.y_min;
      }
      if (event == Event::Home) {
        (hovered_) = 0;
      }
      if (event == Event::End) {
        (hovered_) = size() - 1;
      }
      if (event == Event::Tab && size()) {
        hovered_ = (hovered_ + 1) % size();
      }
      if (event == Event::TabReverse && size()) {
        hovered_ = (hovered_ + size() - 1) % size();
      }

      hovered_ = util::clamp(hovered_, 0, size() - 1);

      if (hovered_ != old_hovered) {
        focused_entry() = hovered_;
        option_->on_change();
        return true;
      }
    }

    if (event == Event::Character(' ') || event == Event::Return) {
      *selected_ = hovered_;
      //*selected_ = focused_entry();
      option_->on_change();
    }

    return false;
  }

  bool OnMouseEvent(const Event& event) noexcept {
    if (event.mouse().button == Mouse::Button::WheelDown ||
        event.mouse().button == Mouse::Button::WheelUp) {
      return OnMouseWheel(event);
    }

    for (int i = 0; i < size(); ++i) {
      if (!boxes_[i].Contain(event.mouse().x, event.mouse().y)) {
        continue;
      }

      TakeFocus();
      focused_entry() = i;
      if (event.mouse().button == Mouse::Button::Left &&
          event.mouse().motion == Mouse::Motion::Released) {
        if (*selected_ != i) {
          *selected_ = i;
          option_->on_change();
        }

        return true;
      }
    }
    return false;
  }

  bool OnMouseWheel(const Event& event) noexcept {
    if (!box_.Contain(event.mouse().x, event.mouse().y)) {
      return false;
    }

    int old_hovered = hovered_;

    if (event.mouse().button == Mouse::Button::WheelUp) {
      (hovered_)--;
    }
    if (event.mouse().button == Mouse::Button::WheelDown) {
      (hovered_)++;
    }

    hovered_ = util::clamp(hovered_, 0, size() - 1);

    if (hovered_ != old_hovered) {
      option_->on_change();
    }

    return true;
  }

  void Clamp() noexcept {
    boxes_.resize(size());
    *selected_ = util::clamp(*selected_, 0, size() - 1);
    focused_entry() = util::clamp(focused_entry(), 0, size() - 1);
    hovered_ = util::clamp(hovered_, 0, size() - 1);
  }

  [[nodiscard]] bool Focusable() const noexcept final {
    return entries_.size();
  }
  int& focused_entry() { return option_->focused_entry(); }
  [[nodiscard]] int size() const { return static_cast<int>(entries_.size()); }

  int* selected_;
  int hovered_{};

  Box box_{};
  Ref<RadioboxOption> option_;

  ConstStringListRef entries_;
  std::vector<Box> boxes_;
};

}  // namespace

/// @brief A list of element, where only one can be selected.
/// @param entries The list of entries in the list.
/// @param selected The index of the currently selected element.
/// @param option Additional optional parameters.
/// @ingroup component
/// @see RadioboxBase
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::TerminalOutput();
/// std::vector<std::string> entries = {
///     "entry 1",
///     "entry 2",
///     "entry 3",
/// };
/// int selected = 0;
/// auto menu = Radiobox(&entries, &selected);
/// screen.Loop(menu);
/// ```
///
/// ### Output
///
/// ```bash
/// ◉ entry 1
/// ○ entry 2
/// ○ entry 3
/// ```
Component Radiobox(ConstStringListRef entries,
                   int* selected,
                   Ref<RadioboxOption> option) noexcept {
  return Make<RadioboxBase>(entries, selected, std::move(option));
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
