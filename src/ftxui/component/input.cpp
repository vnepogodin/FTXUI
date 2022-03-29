#include <algorithm>   // for max, min
#include <cstddef>     // for size_t
#include <functional>  // for function
#include <memory>      // for shared_ptr, allocator
#include <ranges>
#include <string>   // for string, wstring
#include <utility>  // for move
#include <vector>   // for vector

#include <ftxui/component/captured_mouse.hpp>     // for CapturedMouse
#include <ftxui/component/component.hpp>          // for Make, Input
#include <ftxui/component/component_base.hpp>     // for ComponentBase
#include <ftxui/component/component_options.hpp>  // for InputOption
#include <ftxui/component/deprecated.hpp>         // for Input
#include <ftxui/component/event.hpp>  // for Event, Event::ArrowLeft, Event::ArrowRight, Event::Backspace, Event::Custom, Event::Delete, Event::End, Event::Home, Event::Return
#include <ftxui/component/mouse.hpp>  // for Mouse, Mouse::Left, Mouse::Pressed
#include <ftxui/component/screen_interactive.hpp>  // for Component
#include <ftxui/dom/elements.hpp>  // for operator|, text, Element, reflect, inverted, Decorator, flex, focus, hbox, size, bold, dim, frame, select, EQUAL, HEIGHT
#include <ftxui/screen/box.hpp>    // for Box
#include <ftxui/screen/string.hpp>  // for GlyphPosition, GlyphCount, to_string, CellToGlyphIndex, to_wstring
#include <ftxui/screen/util.hpp>  // for clamp
#include <ftxui/util/ref.hpp>  // for StringRef, Ref, WideStringRef, ConstStringRef

namespace ranges = std::ranges;

namespace ftxui {

namespace {

std::string PasswordField(int size) noexcept {
  std::string out;
  out.reserve(2 * size);
  while (size--)
    out += "•";
  return out;
}

// An input box. The user can type text into it.
class InputBase : public ComponentBase {
 public:
  InputBase(StringRef content,
            ConstStringRef placeholder,
            Ref<InputOption> option)
      : content_(std::move(content)),
        placeholder_(std::move(placeholder)),
        option_(std::move(option)) {}

  int cursor_position_internal_ = 0;
  int& cursor_position() noexcept {
    int& opt = option_->cursor_position();
    if (opt != -1)
      return opt;
    return cursor_position_internal_;
  }

  // Component implementation:
  Element Render() noexcept override {
    std::string password_content;
    if (option_->password())
      password_content = PasswordField(static_cast<int>(content_->size()));
    const std::string& content =
        option_->password() ? password_content : *content_;

    const int size = GlyphCount(content);

    cursor_position() = ranges::max(0, ranges::min(size, cursor_position()));
    auto main_decorator = flex | ftxui::size(HEIGHT, EQUAL, 1);
    const bool is_focused = Focused();

    // placeholder.
    if (size == 0) {
      const bool hovered = hovered_;
      Decorator decorator = dim | main_decorator;
      if (is_focused)
        decorator = decorator | focus;
      if (hovered || is_focused)
        decorator = decorator | inverted;
      return text(*placeholder_) | decorator | reflect(box_);
    }

    // Not focused.
    if (!is_focused) {
      if (hovered_)
        return text(content) | main_decorator | inverted | reflect(box_);
      else
        return text(content) | main_decorator | reflect(box_);
    }

    const int index_before_cursor = GlyphPosition(content, cursor_position());
    const int index_after_cursor =
        GlyphPosition(content, 1, index_before_cursor);
    const auto part_before_cursor = content.substr(0, index_before_cursor);
    std::string part_at_cursor = " ";
    if (cursor_position() < size) {
      part_at_cursor = content.substr(index_before_cursor,
                                      index_after_cursor - index_before_cursor);
    }
    const auto part_after_cursor = content.substr(index_after_cursor);
    const auto focused = (is_focused || hovered_) ? focus : select;
    return hbox({
               text(part_before_cursor),
               text(part_at_cursor) | focused | inverted | reflect(cursor_box_),
               text(part_after_cursor),
           }) |
           flex | frame | bold | main_decorator | reflect(box_);
  }

  bool OnEvent(const Event& event) noexcept override {
    cursor_position() = ranges::max(
        0, ranges::min(static_cast<int>(content_->size()), cursor_position()));

    if (event.is_mouse())
      return OnMouseEvent(event);

    std::string c;

    // Backspace.
    if (event == Event::Backspace) {
      if (cursor_position() == 0)
        return false;
      const size_t start = GlyphPosition(*content_, cursor_position() - 1);
      const size_t end = GlyphPosition(*content_, cursor_position());
      content_->erase(start, end - start);
      cursor_position()--;
      option_->on_change();
      return true;
    }

    // Delete
    if (event == Event::Delete) {
      if (cursor_position() == int(content_->size()))
        return false;
      const size_t start = GlyphPosition(*content_, cursor_position());
      const size_t end = GlyphPosition(*content_, cursor_position() + 1);
      content_->erase(start, end - start);
      option_->on_change();
      return true;
    }

    // Enter.
    if (event == Event::Return) {
      option_->on_enter();
      return true;
    }

    if (event == Event::Custom) {
      return false;
    }

    if (event == Event::ArrowLeft && cursor_position() > 0) {
      cursor_position()--;
      return true;
    }

    if (event == Event::ArrowRight &&
        cursor_position() < static_cast<int>(content_->size())) {
      cursor_position()++;
      return true;
    }

    if (event == Event::Home) {
      cursor_position() = 0;
      return true;
    }

    if (event == Event::End) {
      cursor_position() = GlyphCount(*content_);
      return true;
    }

    // Content
    if (event.is_character()) {
      const size_t start = GlyphPosition(*content_, cursor_position());
      content_->insert(start, event.character());
      cursor_position()++;
      option_->on_change();
      return true;
    }
    return false;
  }

 private:
  bool OnMouseEvent(const Event& event) noexcept {
    hovered_ =
        box_.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);
    if (!hovered_)
      return false;

    if (event.mouse().button != Mouse::Button::Left ||
        event.mouse().motion != Mouse::Motion::Pressed) {
      return false;
    }

    TakeFocus();
    if (content_->empty())
      return true;

    const auto mapping = CellToGlyphIndex(*content_);
    int original_glyph = cursor_position();
    original_glyph = util::clamp(original_glyph, 0, int(mapping.size()));
    int original_cell = 0;
    for (size_t i = 0; i < mapping.size(); i++) {
      if (mapping[i] == original_glyph) {
        original_cell = i;
        break;
      }
    }
    if (mapping[original_cell] != original_glyph)
      original_cell = static_cast<int>(mapping.size());
    const int target_cell = original_cell + event.mouse().x - cursor_box_.x_min;
    int target_glyph = target_cell < static_cast<int>(mapping.size())
                           ? mapping[target_cell]
                           : static_cast<int>(mapping.size());
    target_glyph = util::clamp(target_glyph, 0, GlyphCount(*content_));
    if (cursor_position() != target_glyph) {
      cursor_position() = target_glyph;
      option_->on_change();
    }
    return true;
  }

  [[nodiscard]] bool Focusable() const noexcept final { return true; }

  bool hovered_{false};
  StringRef content_;
  ConstStringRef placeholder_;

  Box box_;
  Box cursor_box_;
  Ref<InputOption> option_;
};

// An input box. The user can type text into it.
// For convenience, the std::wstring version of Input simply wrap a
// InputBase.
class WideInputBase : public InputBase {
 public:
  WideInputBase(WideStringRef content,
                ConstStringRef placeholder,
                Ref<InputOption> option)
      : InputBase(StringRef{&wrapped_content_},
                  std::move(placeholder),
                  std::move(option)),
        content_(std::move(content)),
        wrapped_content_(to_string(*content_)) {}

  Element Render() noexcept override {
    wrapped_content_ = ftxui::to_string(*content_);
    return InputBase::Render();
  }

  bool OnEvent(const Event& event) noexcept override {
    wrapped_content_ = to_string(*content_);
    if (InputBase::OnEvent(event)) {
      *content_ = to_wstring(wrapped_content_);
      return true;
    }
    return false;
  }

  WideStringRef content_;
  std::string wrapped_content_{};
};

}  // namespace

/// @brief An input box for editing text.
/// @param content The editable content.
/// @param placeholder The text displayed when content is still empty.
/// @param option Additional optional parameters.
/// @ingroup component
/// @see InputBase
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::FitComponent();
/// std::string content= "";
/// std::string placeholder = "placeholder";
/// Component input = Input(&content, &placeholder);
/// screen.Loop(input);
/// ```
///
/// ### Output
///
/// ```bash
/// placeholder
/// ```
Component Input(const StringRef& content,
                const ConstStringRef& placeholder,
                Ref<InputOption> option) noexcept {
  return Make<InputBase>(content, placeholder, std::move(option));
}

/// @brief . An input box for editing text.
/// @param content The editable content.
/// @param placeholder The text displayed when content is still empty.
/// @param option Additional optional parameters.
/// @ingroup component
/// @see InputBase
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::FitComponent();
/// std::string content= "";
/// std::string placeholder = "placeholder";
/// Component input = Input(&content, &placeholder);
/// screen.Loop(input);
/// ```
///
/// ### Output
///
/// ```bash
/// placeholder
/// ```
Component Input(const WideStringRef& content,
                const ConstStringRef& placeholder,
                Ref<InputOption> option) noexcept {
  return Make<WideInputBase>(content, placeholder, std::move(option));
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
