#ifndef FTXUI_DOM_CANVAS_HPP
#define FTXUI_DOM_CANVAS_HPP

#include <stddef.h>       // for size_t
#include <functional>     // for function
#include <string>         // for string
#include <unordered_map>  // for unordered_map

#include "ftxui/screen/color.hpp"   // for Color
#include "ftxui/screen/screen.hpp"  // for Pixel

namespace ftxui {

struct Canvas {
 public:
  constexpr Canvas() noexcept = default;
  Canvas(int width, int height) noexcept;

  // Getters:
  constexpr int width() const noexcept { return width_; }
  constexpr int height() const noexcept { return height_; }
  Pixel GetPixel(int x, int y) const noexcept;

  using Stylizer = std::function<void(Pixel&)>;

  // Draws using braille characters --------------------------------------------
  void DrawPointOn(int x, int y) noexcept;
  void DrawPointOff(int x, int y) noexcept;
  void DrawPointToggle(int x, int y) noexcept;
  void DrawPoint(int x, int y, bool value) noexcept;
  void DrawPoint(int x, int y, bool value, const Stylizer& s) noexcept;
  void DrawPoint(int x, int y, bool value, const Color& color) noexcept;
  void DrawPointLine(int x1, int y1, int x2, int y2) noexcept;
  void DrawPointLine(int x1, int y1, int x2, int y2, const Stylizer& s) noexcept;
  void DrawPointLine(int x1, int y1, int x2, int y2, const Color& color) noexcept;
  void DrawPointCircle(int x, int y, int radius) noexcept;
  void DrawPointCircle(int x, int y, int radius, const Stylizer& s) noexcept;
  void DrawPointCircle(int x, int y, int radius, const Color& color) noexcept;
  void DrawPointCircleFilled(int x, int y, int radius) noexcept;
  void DrawPointCircleFilled(int x, int y, int radius, const Stylizer& s) noexcept;
  void DrawPointCircleFilled(int x, int y, int radius, const Color& color) noexcept;
  void DrawPointEllipse(int x, int y, int r1, int r2) noexcept;
  void DrawPointEllipse(int x, int y, int r1, int r2, const Color& color) noexcept;
  void DrawPointEllipse(int x, int y, int r1, int r2, const Stylizer& s) noexcept;
  void DrawPointEllipseFilled(int x, int y, int r1, int r2) noexcept;
  void DrawPointEllipseFilled(int x, int y, int r1, int r2, const Color& color) noexcept;
  void DrawPointEllipseFilled(int x, int y, int r1, int r2, const Stylizer& s) noexcept;

  // Draw using box characters -------------------------------------------------
  // Block are of size 1x2. y is considered to be a multiple of 2.
  void DrawBlockOn(int x, int y) noexcept;
  void DrawBlockOff(int x, int y) noexcept;
  void DrawBlockToggle(int x, int y) noexcept;
  void DrawBlock(int x, int y, bool value) noexcept;
  void DrawBlock(int x, int y, bool value, const Stylizer& s) noexcept;
  void DrawBlock(int x, int y, bool value, const Color& color) noexcept;
  void DrawBlockLine(int x1, int y1, int x2, int y2) noexcept;
  void DrawBlockLine(int x1, int y1, int x2, int y2, const Stylizer& s) noexcept;
  void DrawBlockLine(int x1, int y1, int x2, int y2, const Color& color) noexcept;
  void DrawBlockCircle(int x1, int y1, int radius) noexcept;
  void DrawBlockCircle(int x1, int y1, int radius, const Stylizer& s) noexcept;
  void DrawBlockCircle(int x1, int y1, int radius, const Color& color) noexcept;
  void DrawBlockCircleFilled(int x1, int y1, int radius) noexcept;
  void DrawBlockCircleFilled(int x1, int y1, int radius, const Stylizer& s) noexcept;
  void DrawBlockCircleFilled(int x1, int y1, int radius, const Color& color) noexcept;
  void DrawBlockEllipse(int x1, int y1, int r1, int r2) noexcept;
  void DrawBlockEllipse(int x1, int y1, int r1, int r2, const Stylizer& s) noexcept;
  void DrawBlockEllipse(int x1, int y1, int r1, int r2, const Color& color) noexcept;
  void DrawBlockEllipseFilled(int x1, int y1, int r1, int r2) noexcept;
  void DrawBlockEllipseFilled(int x1,
                              int y1,
                              int r1,
                              int r2,
                              const Stylizer& s) noexcept;
  void DrawBlockEllipseFilled(int x1,
                              int y1,
                              int r1,
                              int r2,
                              const Color& color) noexcept;

  // Draw using normal characters ----------------------------------------------
  // Draw using character of size 2x4 at position (x,y)
  // x is considered to be a multiple of 2.
  // y is considered to be a multiple of 4.
  void DrawText(int x, int y, const std::string& value) noexcept;
  void DrawText(int x, int y, const std::string& value, const Color& color) noexcept;
  void DrawText(int x, int y, const std::string& value, const Stylizer& style) noexcept;

  // Decorator:
  // x is considered to be a multiple of 2.
  // y is considered to be a multiple of 4.
  void Style(int x, int y, const Stylizer& style) noexcept;

 private:
  constexpr bool IsIn(int x, int y) const noexcept {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
  }
  enum CellType {
    kBraille,
    kBlock,
    kText,
  };
  struct Cell {
    CellType type = kText;
    Pixel content;
  };
  struct XY {
    int x;
    int y;
    constexpr bool operator==(const XY& other) const noexcept {
      return x == other.x && y == other.y;
    }
  };

  struct XYHash {
    constexpr size_t operator()(const XY& xy) const noexcept {
      return static_cast<size_t>(xy.x * 1024 + xy.y);
    }
  };

  int width_ = 0;
  int height_ = 0;
  std::unordered_map<XY, Cell, XYHash> storage_{};
};

}  // namespace ftxui

#endif  // FTXUI_DOM_CANVAS_HPP

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
