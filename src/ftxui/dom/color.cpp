#include <memory>   // for make_unique
#include <utility>  // for move

#include "ftxui/dom/elements.hpp"  // for Element, Decorator, bgcolor, color
#include "ftxui/dom/node_decorator.hpp"  // for NodeDecorator
#include "ftxui/screen/box.hpp"          // for Box
#include "ftxui/screen/color.hpp"        // for Color
#include "ftxui/screen/screen.hpp"       // for Pixel, Screen

namespace ftxui {

class BgColor : public NodeDecorator {
 public:
  BgColor(const Element& child, const Color& color)
      : NodeDecorator(child), color_(color) {}

  void Render(Screen& screen) noexcept override {
    for (int y = box_.y_min; y <= box_.y_max; ++y) {
      for (int x = box_.x_min; x <= box_.x_max; ++x) {
        screen.PixelAt(x, y).background_color = color_;
      }
    }
    NodeDecorator::Render(screen);
  }

  Color color_;
};

class FgColor : public NodeDecorator {
 public:
  FgColor(const Element& child, const Color& color)
      : NodeDecorator(child), color_(color) {}

  void Render(Screen& screen) noexcept override {
    for (int y = box_.y_min; y <= box_.y_max; ++y) {
      for (int x = box_.x_min; x <= box_.x_max; ++x) {
        screen.PixelAt(x, y).foreground_color = color_;
      }
    }
    NodeDecorator::Render(screen);
  }

  Color color_;
};

/// @brief Set the foreground color of an element.
/// @param color The color of the output element.
/// @param child The input element.
/// @return The output element colored.
/// @ingroup dom
///
/// ### Example
///
/// ```cpp
/// Element document = color(Color::Green, text("Success")),
/// ```
Element color(const Color& color, const Element& child) noexcept {
  return std::make_unique<FgColor>(child, color);
}

/// @brief Set the background color of an element.
/// @param color The color of the output element.
/// @param child The input element.
/// @return The output element colored.
/// @ingroup dom
///
/// ### Example
///
/// ```cpp
/// Element document = bgcolor(Color::Green, text("Success")),
/// ```
Element bgcolor(const Color& color, const Element& child) noexcept {
  return std::make_unique<BgColor>(child, color);
}

/// @brief Decorate using a foreground color.
/// @param c The foreground color to be applied.
/// @return The Decorator applying the color.
/// @ingroup dom
///
/// ### Example
///
/// ```cpp
/// Element document = text("red") | color(Color::Red);
/// ```
[[gnu::const]]
Decorator color(Color c) noexcept {
  return [c](auto&& child) { return color(c, std::forward<decltype(child)>(child)); };
}

/// @brief Decorate using a background color.
/// @param color The background color to be applied.
/// @return The Decorator applying the color.
/// @ingroup dom
///
/// ### Example
///
/// ```cpp
/// Element document = text("red") | bgcolor(Color::Red);
/// ```
[[gnu::const]]
Decorator bgcolor(Color color) noexcept {
  return [color](auto&& child) { return bgcolor(color, std::forward<decltype(child)>(child)); };
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
