#include <algorithm>    // for min
#include <memory>       // for make_shared
#include <string>       // for string, wstring
#include <string_view>  // for string_view
#include <vector>       // for vector

#include "ftxui/dom/deprecated.hpp"   // for text, vtext
#include "ftxui/dom/elements.hpp"     // for Element, text, vtext
#include "ftxui/dom/node.hpp"         // for Node
#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box
#include "ftxui/screen/screen.hpp"    // for Pixel, Screen
#include "ftxui/screen/string.hpp"  // for string_width, Utf8ToGlyphs, to_string

namespace ftxui {

using ftxui::Screen;

class Text : public Node {
 public:
  Text(const char* text) : text_(text) {}
  Text(const std::string text) : text_(text) {}
  Text(const std::string_view text) : text_(text) {}

  void ComputeRequirement() override {
    requirement_.min_x = string_width(text_);
    requirement_.min_y = 1;
  }

  void Render(Screen& screen) override {
    int x = box_.x_min;
    const int& y = box_.y_min;
    if (y > box_.y_max)
      return;
    for (const auto& cell : Utf8ToGlyphs(text_)) {
      if (x > box_.x_max)
        return;
      screen.PixelAt(x, y).character = cell;
      ++x;
    }
  }

 private:
  const std::string text_{};
};

class VText : public Node {
 public:
  VText(std::string text)
      : text_(text), width_{std::min(string_width(text_), 1)} {}

  void ComputeRequirement() override {
    requirement_.min_x = width_;
    requirement_.min_y = string_width(text_);
  }

  void Render(Screen& screen) override {
    const int& x = box_.x_min;
    int y = box_.y_min;
    if (x + width_ - 1 > box_.x_max)
      return;
    for (const auto& it : Utf8ToGlyphs(text_)) {
      if (y > box_.y_max)
        return;
      screen.PixelAt(x, y).character = it;
      y += 1;
    }
  }

 private:
  std::string text_;
  int width_ = 1;
};

/// @brief Display a piece of UTF8 encoded unicode text.
/// @ingroup dom
/// @see ftxui::to_wstring
///
/// ### Example
///
/// ```cpp
/// Element document = text("Hello world!");
/// ```
///
/// ### Output
///
/// ```bash
/// Hello world!
/// ```
Element text(const std::string text) {
  return std::make_shared<Text>(text);
}

Element text(const std::string_view text) {
  return std::make_shared<Text>(text);
}

Element text(const char* text) {
  return std::make_shared<Text>(text);
}

/// @brief Display a piece of unicode text.
/// @ingroup dom
/// @see ftxui::to_wstring
///
/// ### Example
///
/// ```cpp
/// Element document = text(L"Hello world!");
/// ```
///
/// ### Output
///
/// ```bash
/// Hello world!
/// ```
Element text(std::wstring text) {
  return std::make_shared<Text>(to_string(text));
}

/// @brief Display a piece of unicode text vertically.
/// @ingroup dom
/// @see ftxui::to_wstring
///
/// ### Example
///
/// ```cpp
/// Element document = vtext("Hello world!");
/// ```
///
/// ### Output
///
/// ```bash
/// H
/// e
/// l
/// l
/// o
///
/// w
/// o
/// r
/// l
/// d
/// !
/// ```
Element vtext(std::string text) {
  return std::make_shared<VText>(text);
}

/// @brief Display a piece unicode text vertically.
/// @ingroup dom
/// @see ftxui::to_wstring
///
/// ### Example
///
/// ```cpp
/// Element document = vtext(L"Hello world!");
/// ```
///
/// ### Output
///
/// ```bash
/// H
/// e
/// l
/// l
/// o
///
/// w
/// o
/// r
/// l
/// d
/// !
/// ```
Element vtext(std::wstring text) {
  return std::make_shared<VText>(to_string(text));
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
