#ifndef FTXUI_COMPONENT_EVENT_HPP
#define FTXUI_COMPONENT_EVENT_HPP

#include <ftxui/component/mouse.hpp>  // for Mouse
#include <ftxui/screen/string.hpp>    // for to_wstring
#include <functional>
#include <string>  // for string, operator==
#include <vector>
#include <variant>

namespace ftxui {

class ScreenInteractive;
class ComponentBase;

/// @brief Represent an event. It can be key press event, a terminal resize, or
/// more ...
///
/// For example:
/// - Printable character can be created using Event::Character('a').
/// - Some special are predefined, like Event::ArrowLeft.
/// - One can find arbitrary code for special Events using:
///   ./example/util/print_key_press
///  For instance, CTLR+A maps to Event::Special({1});
///
/// Useful documentation about xterm specification:
/// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
struct Event {
  struct Cursor {
    int x{};
    int y{};
  };
  enum class Type {
    Unknown,
    Character,
    Mouse,
    CursorReporting,
  };

  Event() = default;
  Event(const Event&) = default;
  Event(Event&&) = default;
  Event& operator=(const Event&) = default;
  Event& operator=(Event&&) = default;
  ~Event() = default;

  // --- Constructor section ---------------------------------------------------
  //static Event Character(const std::string&) noexcept;
  static Event Character(const std::string& input) noexcept {
    Event event;
    event.type_ = Type::Character;
    event.input_ = input;
    event.device_ = Cursor{};
    return event;
  }
  static inline Event Character(char ch) noexcept {
    return Event::Character(std::string{ch});
  };
  [[maybe_unused]] static inline Event Character(wchar_t ch) noexcept {
    return Event::Character(ftxui::to_string(std::wstring{ch}));
  };
  static inline Event Special(const std::string& input) noexcept {
    Event event;
    event.input_ = input;
    return event;
  }
  static inline Event Mouse(const std::string& input, struct Mouse mouse) noexcept {
    Event event;
    event.type_ = Type::Mouse;
    event.input_ = input;
    event.device_ = mouse;
    return event;
  }
  static Event CursorReporting(const std::string&, int x, int y) noexcept;

  // --- Arrow ---
  static const Event ArrowLeft;
  static const Event ArrowRight;
  static const Event ArrowUp;
  static const Event ArrowDown;

  // --- Other ---
  static const Event Backspace;
  static const Event Delete;
  static const Event Return;
  static const Event Escape;
  static const Event Tab;
  static const Event TabReverse;
  static const Event F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12;

  static const Event Home;
  static const Event End;

  static const Event PageUp;
  static const Event PageDown;

  // --- Custom ---
  static const Event Custom;

  //--- Method section ---------------------------------------------------------
  [[nodiscard]] constexpr inline bool is_character() const noexcept
  { return type_ == Type::Character; }
  [[nodiscard]] inline std::string character() const noexcept { return input_; }

  [[nodiscard]] constexpr inline bool is_mouse() const noexcept { return type_ == Type::Mouse; }
  struct Mouse& mouse() noexcept
  { return std::get<struct Mouse>(device_); }

  [[nodiscard]] constexpr const struct Mouse& mouse() const noexcept
  { return std::get<struct Mouse>(device_); }
  struct Cursor& cursor() noexcept
  { return std::get<struct Cursor>(device_); }

  [[nodiscard]] constexpr struct Cursor& cursor() const noexcept
  { return const_cast<Cursor&>(std::get<struct Cursor>(device_)); }

  [[nodiscard]] constexpr bool is_cursor_reporting() const noexcept
  { return type_ == Type::CursorReporting; }
  [[nodiscard]] constexpr inline int cursor_x() const noexcept { return cursor().x; }
  [[nodiscard]] constexpr inline int cursor_y() const noexcept { return cursor().y; }

  [[nodiscard]] const std::string& input() const noexcept { return input_; }

  inline bool operator==(const Event& other) const noexcept {
    return input_ == other.input_;
  }
  inline bool operator!=(const Event& other) const noexcept {
    return !operator==(other);
  }

  //--- State section ----------------------------------------------------------
  ScreenInteractive* screen_ = nullptr;

 //private:
  friend ComponentBase;
  friend ScreenInteractive;

  Type type_{Type::Unknown};

  std::variant<struct Mouse, struct Cursor> device_{};
  std::string input_{};
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_EVENT_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
