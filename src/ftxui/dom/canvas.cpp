#include <ftxui/dom/canvas.hpp>

#include <cstdint>  // for uint8_t
#include <cstdlib>  // for abs

#include <array>
#include <memory>  // for make_shared
#include <unordered_map>  // for allocator, unordered_map
#include <utility>        // for move, pair
#include <vector>         // for vector

#include <ftxui/dom/elements.hpp>     // for Element, canvas
#include <ftxui/dom/node.hpp>         // for Node
#include <ftxui/dom/requirement.hpp>  // for Requirement
#include <ftxui/screen/box.hpp>       // for Box
#include <ftxui/screen/screen.hpp>    // for Pixel, Screen
#include <ftxui/screen/string.hpp>    // for Utf8ToGlyphs
#include <ftxui/util/ref.hpp>         // for ConstRef

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

namespace {

// Base UTF8 pattern:
// 11100010 10100000 10000000 // empty

// Pattern for the individuel dots:
// ┌──────┬───────┐
// │dot1  │ dot4  │
// ├──────┼───────┤
// │dot2  │ dot5  │
// ├──────┼───────┤
// │dot3  │ dot6  │
// ├──────┼───────┤
// │dot0-1│ dot0-2│
// └──────┴───────┘
// 11100010 10100000 10000001 // dot1
// 11100010 10100000 10000010 // dot2
// 11100010 10100000 10000100 // dot3
// 11100010 10100001 10000000 // dot0-1
// 11100010 10100000 10001000 // dot4
// 11100010 10100000 10010000 // dot5
// 11100010 10100000 10100000 // dot6
// 11100010 10100010 10000000 // dot0-2

// NOLINTNEXTLINE
constexpr uint8_t g_map_braille[2][4][2] = {
    {
        {0b00000000, 0b00000001},  // NOLINT | dot1
        {0b00000000, 0b00000010},  // NOLINT | dot2
        {0b00000000, 0b00000100},  // NOLINT | dot3
        {0b00000001, 0b00000000},  // NOLINT | dot0-1
    },
    {
        {0b00000000, 0b00001000},  // NOLINT | dot4
        {0b00000000, 0b00010000},  // NOLINT | dot5
        {0b00000000, 0b00100000},  // NOLINT | dot6
        {0b00000010, 0b00000000},  // NOLINT | dot0-2
    },
};

// NOLINTNEXTLINE
const std::array<std::string, 16> g_map_block = {
    " ", "▘", "▖", "▌", "▝", "▀", "▞", "▛",
    "▗", "▚", "▄", "▙", "▐", "▜", "▟", "█",
};

// NOLINTNEXTLINE
const std::unordered_map<std::string, uint8_t> g_map_block_inversed = {
    {" ", 0b0000}, {"▘", 0b0001}, {"▖", 0b0010}, {"▌", 0b0011},
    {"▝", 0b0100}, {"▀", 0b0101}, {"▞", 0b0110}, {"▛", 0b0111},
    {"▗", 0b1000}, {"▚", 0b1001}, {"▄", 0b1010}, {"▙", 0b1011},
    {"▐", 0b1100}, {"▜", 0b1101}, {"▟", 0b1110}, {"█", 0b1111},
};

constexpr auto nostyle = [](Pixel& /*pixel*/) {};

}  // namespace

/// @brief Constructor.
/// @param width the width of the canvas. A cell is a 2x8 braille dot.
/// @param height the height of the canvas. A cell is a 2x8 braille dot.
Canvas::Canvas(int width, int height) noexcept
: width_(width), height_(height), storage_(static_cast<size_t>(width_ * height_ / 8)) {}

/// @brief Get the content of a cell.
/// @param x the x coordinate of the cell.
/// @param y the y coordinate of the cell.
Pixel Canvas::GetPixel(int x, int y) const noexcept {
const auto it = storage_.find(XY{x, y});
return (it == storage_.end()) ? Pixel{} : it->second.content;
}

/// @brief Draw a braille dot.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
/// @param value whether the dot is filled or not.
void Canvas::DrawPoint(int x, int y, bool value) noexcept {
DrawPoint(x, y, value, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a braille dot.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
/// @param value whether the dot is filled or not.
/// @param color the color of the dot.
void Canvas::DrawPoint(int x, int y, bool value, const Color& color) noexcept {
DrawPoint(x, y, value, [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a braille dot.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
/// @param value whether the dot is filled or not.
/// @param style the style of the cell.
void Canvas::DrawPoint(int x,
                   int y,
                   bool value,
                   const Stylizer& style) noexcept {
Style(x, y, style);
if (value)
DrawPointOn(x, y);
else
DrawPointOff(x, y);
}

/// @brief Draw a braille dot.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
void Canvas::DrawPointOn(int x, int y) noexcept {
if (!IsIn(x, y))
return;
Cell& cell = storage_[XY{x / 2, y / 4}];
if (cell.type != CellType::kBraille) {
cell.content.character = "⠀";  // 3 bytes.
cell.type = CellType::kBraille;
}

cell.content.character[1] |= g_map_braille[x % 2][y % 4][0];
cell.content.character[2] |= g_map_braille[x % 2][y % 4][1];
}

/// @brief Erase a braille dot.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
void Canvas::DrawPointOff(int x, int y) noexcept {
if (!IsIn(x, y))
return;
Cell& cell = storage_[XY{x / 2, y / 4}];
if (cell.type != CellType::kBraille) {
    cell.content.character = "⠀";  // 3 byt
    cell.type = CellType::kBraille;
  }

  cell.content.character[1] &= ~(g_map_braille[x % 2][y % 4][0]);  // NOLINT
  cell.content.character[2] &= ~(g_map_braille[x % 2][y % 4][1]);  // NOLINT
}

/// @brief Toggle a braille dot. A filled one will be erased, and the other will
/// be drawn.
/// @param x the x coordinate of the dot.
/// @param y the y coordinate of the dot.
void Canvas::DrawPointToggle(int x, int y) noexcept {
  if (!IsIn(x, y)) {
    return;
  }
  Cell& cell = storage_[XY{x / 2, y / 4}];
  if (cell.type != CellType::kBraille) {
    cell.content.character = "⠀";  // 3 byt
    cell.type = CellType::kBraille;
  }

  cell.content.character[1] ^= g_map_braille[x % 2][y % 4][0];  // NOLINT
  cell.content.character[2] ^= g_map_braille[x % 2][y % 4][1];  // NOLINT
}

/// @brief Draw a line made of braille dots.
/// @param x1 the x coordinate of the first dot.
/// @param y1 the y coordinate of the first dot.
/// @param x2 the x coordinate of the second dot.
/// @param y2 the y coordinate of the second dot.
void Canvas::DrawPointLine(int x1, int y1, int x2, int y2) noexcept {
  DrawPointLine(x1, y1, x2, y2, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a line made of braille dots.
/// @param x1 the x coordinate of the first dot.
/// @param y1 the y coordinate of the first dot.
/// @param x2 the x coordinate of the second dot.
/// @param y2 the y coordinate of the second dot.
/// @param color the color of the line.
void Canvas::DrawPointLine(int x1,
                           int y1,
                           int x2,
                           int y2,
                           const Color& color) noexcept {
  DrawPointLine(x1, y1, x2, y2,
                [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a line made of braille dots.
/// @param x1 the x coordinate of the first dot.
/// @param y1 the y coordinate of the first dot.o
/// @param x2 the x coordinate of the second dot.
/// @param y2 the y coordinate of the second dot.
/// @param style the style of the line.
void Canvas::DrawPointLine(int x1,
                           int y1,
                           int x2,
                           int y2,
                           const Stylizer& style) noexcept {
  const int dx = std::abs(x2 - x1);
  const int dy = std::abs(y2 - y1);
  const int sx = x1 < x2 ? 1 : -1;
  const int sy = y1 < y2 ? 1 : -1;
  const int length = ranges::max(dx, dy);

  if (!IsIn(x1, y1) && !IsIn(x2, y2)) {
    return;
  }
  if (dx + dx > width_ * height_) {
    return;
  }

  int error = dx - dy;
  for (int i = 0; i < length; ++i) {
    DrawPoint(x1, y1, true, style);
    if (2 * error >= -dy) {
      error -= dy;
      x1 += sx;
    }
    if (2 * error <= dx) {
      error += dx;
      y1 += sy;
    }
  }
  DrawPoint(x2, y2, true, style);
}

/// @brief Draw a circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
void Canvas::DrawPointCircle(int x, int y, int radius) noexcept {
  DrawPointCircle(x, y, radius, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param color the color of the circle.
void Canvas::DrawPointCircle(int x,
                             int y,
                             int radius,
                             const Color& color) noexcept {
  DrawPointCircle(x, y, radius,
                  [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param style the style of the circle.
void Canvas::DrawPointCircle(int x,
                             int y,
                             int radius,
                             const Stylizer& style) noexcept {
  DrawPointEllipse(x, y, radius, radius, style);
}

/// @brief Draw a filled circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
void Canvas::DrawPointCircleFilled(int x, int y, int radius) noexcept {
  DrawPointCircleFilled(x, y, radius, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a filled circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param color the color of the circle.
void Canvas::DrawPointCircleFilled(int x,
                                   int y,
                                   int radius,
                                   const Color& color) noexcept {
  DrawPointCircleFilled(x, y, radius,
                        [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a filled circle made of braille dots.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param style the style of the circle.
void Canvas::DrawPointCircleFilled(int x,
                                   int y,
                                   int radius,
                                   const Stylizer& style) noexcept {
  DrawPointEllipseFilled(x, y, radius, radius, style);
}

/// @brief Draw an ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
void Canvas::DrawPointEllipse(int x, int y, int r1, int r2) noexcept {
  DrawPointEllipse(x, y, r1, r2, [](Pixel& /*pixel*/) {});
}

/// @brief Draw an ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param color the color of the ellipse.
void Canvas::DrawPointEllipse(int x,
                              int y,
                              int r1,
                              int r2,
                              const Color& color) noexcept {
  DrawPointEllipse(x, y, r1, r2,
                   [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw an ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param style the style of the ellipse.
void Canvas::DrawPointEllipse(int x1,
                              int y1,
                              int r1,
                              int r2,
                              const Stylizer& s) noexcept {
  int x = -r1;
  int y = 0;
  int e2 = r2;
  int dx = (1 + 2 * x) * e2 * e2;
  int dy = x * x;
  int err = dx + dy;

  do {
    DrawPoint(x1 - x, y1 + y, true, s);
    DrawPoint(x1 + x, y1 + y, true, s);
    DrawPoint(x1 + x, y1 - y, true, s);
    DrawPoint(x1 - x, y1 - y, true, s);
    e2 = 2 * err;
    if (e2 >= dx) {
      x++;
      err += dx += 2 * r2 * r2;
    }
    if (e2 <= dy) {
      y++;
      err += dy += 2 * r1 * r1;
    }
  } while (x <= 0);

  while (y++ < r2) {
    DrawPoint(x1, y1 + y, true, s);
    DrawPoint(x1, y1 - y, true, s);
  }
}

/// @brief Draw a filled ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
void Canvas::DrawPointEllipseFilled(int x1, int y1, int r1, int r2) noexcept {
  DrawPointEllipseFilled(x1, y1, r1, r2, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a filled ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param color the color of the ellipse.
void Canvas::DrawPointEllipseFilled(int x1,
                                    int y1,
                                    int r1,
                                    int r2,
                                    const Color& color) noexcept {
  DrawPointEllipseFilled(x1, y1, r1, r2,
                         [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a filled ellipse made of braille dots.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param style the style of the ellipse.
void Canvas::DrawPointEllipseFilled(int x1,
                                    int y1,
                                    int r1,
                                    int r2,
                                    const Stylizer& s) noexcept {
  int x = -r1;
  int y = 0;
  int e2 = r2;
  int dx = (1 + (2 * x)) * e2 * e2;
  int dy = x * x;
  int err = dx + dy;

  do {
    for (int xx = x1 + x; xx <= (x1 - x); ++xx) {
      DrawPoint(xx, y1 + y, true, s);
      DrawPoint(xx, y1 - y, true, s);
    }
    e2 = 2 * err;
    if (e2 >= dx) {
      x++;
      err += dx += static_cast<int>(2L * static_cast<long>(r2) * r2);
    }
    if (e2 <= dy) {
      y++;
      err += dy += static_cast<int>(2L * static_cast<long>(r1) * r1);
    }
  } while (x <= 0);

  while (y++ < r2) {
    for (int yy = y1 - y; yy <= y1 + y; ++yy) {
      DrawPoint(x1, yy, true, s);
    }
  }
}

/// @brief Draw a block.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
/// @param value whether the block is filled or not.
void Canvas::DrawBlock(int x, int y, bool value) noexcept {
  DrawBlock(x, y, value, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a block.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
/// @param value whether the block is filled or not.
/// @param color the color of the block.
void Canvas::DrawBlock(int x, int y, bool value, const Color& color) noexcept {
  DrawBlock(x, y, value, [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a block.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
/// @param value whether the block is filled or not.
/// @param style the style of the block.
void Canvas::DrawBlock(int x,
                       int y,
                       bool value,
                       const Stylizer& style) noexcept {
  Style(x, y, style);
  if (value) {
    DrawBlockOn(x, y);
  } else {
    DrawBlockOff(x, y);
  }
}

/// @brief Draw a block.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
void Canvas::DrawBlockOn(int x, int y) noexcept {
  if (!IsIn(x, y)) {
    return;
  }
  y /= 2;
  Cell& cell = storage_[XY{x / 2, y / 2}];
  if (cell.type != CellType::kBlock) {
    cell.content.character = " ";
    cell.type = CellType::kBlock;
  }

  const uint8_t bit = (x % 2) * 2 + y % 2;
  uint8_t value = g_map_block_inversed.at(cell.content.character);
  value |= 1U << bit;
  cell.content.character = g_map_block[value];
}

/// @brief Erase a block.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
void Canvas::DrawBlockOff(int x, int y) noexcept {
  if (!IsIn(x, y)) {
    return;
  }
  Cell& cell = storage_[XY{x / 2, y / 4}];
  if (cell.type != CellType::kBlock) {
    cell.content.character = " ";
    cell.type = CellType::kBlock;
  }
  y /= 2;

  const uint8_t bit = (y % 2) * 2 + x % 2;
  uint8_t value = g_map_block_inversed.at(cell.content.character);
  value &= ~(1U << bit);
  cell.content.character = g_map_block[value];
}

/// @brief Toggle a block. If it is filled, it will be erased. If it is empty,
/// it will be filled.
/// @param x the x coordinate of the block.
/// @param y the y coordinate of the block.
void Canvas::DrawBlockToggle(int x, int y) noexcept {
  if (!IsIn(x, y)) {
    return;
  }
  Cell& cell = storage_[XY{x / 2, y / 4}];
  if (cell.type != CellType::kBlock) {
    cell.content.character = " ";
    cell.type = CellType::kBlock;
  }
  y /= 2;

  const uint8_t bit = (y % 2) * 2 + x % 2;
  uint8_t value = g_map_block_inversed.at(cell.content.character);
  value ^= 1U << bit;
  cell.content.character = g_map_block[value];
}

/// @brief Draw a line made of block characters.
/// @param x1 the x coordinate of the first point of the line.
/// @param y1 the y coordinate of the first point of the line.
/// @param x2 the x coordinate of the second point of the line.
/// @param y2 the y coordinate of the second point of the line.
void Canvas::DrawBlockLine(int x1, int y1, int x2, int y2) noexcept {
  DrawBlockLine(x1, y1, x2, y2, [](Pixel& /*pixel*/) {});
}

/// @brief Draw a line made of block characters.
/// @param x1 the x coordinate of the first point of the line.
/// @param y1 the y coordinate of the first point of the line.
/// @param x2 the x coordinate of the second point of the line.
/// @param y2 the y coordinate of the second point of the line.
/// @param color the color of the line.
void Canvas::DrawBlockLine(int x1,
                           int y1,
                           int x2,
                           int y2,
                           const Color& color) noexcept {
  DrawBlockLine(x1, y1, x2, y2,
                [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a line made of block characters.
/// @param x1 the x coordinate of the first point of the line.
/// @param y1 the y coordinate of the first point of the line.
/// @param x2 the x coordinate of the second point of the line.
/// @param y2 the y coordinate of the second point of the line.
/// @param style the style of the line.
void Canvas::DrawBlockLine(int x1,
                           int y1,
                           int x2,
                           int y2,
                           const Stylizer& style) noexcept {
  y1 /= 2;
  y2 /= 2;

  const int dx = std::abs(x2 - x1);
  const int dy = std::abs(y2 - y1);
  const int sx = x1 < x2 ? 1 : -1;
  const int sy = y1 < y2 ? 1 : -1;
  const int length = ranges::max(dx, dy);

  if (!IsIn(x1, y1) && !IsIn(x2, y2)) {
    return;
  }
  if (dx + dx > width_ * height_) {
    return;
  }

  int error = dx - dy;
  for (int i = 0; i < length; ++i) {
    DrawBlock(x1, y1 * 2, true, style);
    if (2 * error >= -dy) {
      error -= dy;
      x1 += sx;
    }
    if (2 * error <= dx) {
      error += dx;
      y1 += sy;
    }
  }
  DrawBlock(x2, y2 * 2, true, style);
}

/// @brief Draw a circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
void Canvas::DrawBlockCircle(int x, int y, int radius) noexcept {
  DrawBlockCircle(x, y, radius, nostyle);
}

/// @brief Draw a circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param color the color of the circle.
void Canvas::DrawBlockCircle(int x,
                             int y,
                             int radius,
                             const Color& color) noexcept {
  DrawBlockCircle(x, y, radius,
                  [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param style the style of the circle.
void Canvas::DrawBlockCircle(int x,
                             int y,
                             int radius,
                             const Stylizer& style) noexcept {
  DrawBlockEllipse(x, y, radius, radius, style);
}

/// @brief Draw a filled circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
void Canvas::DrawBlockCircleFilled(int x, int y, int radius) noexcept {
  DrawBlockCircleFilled(x, y, radius, nostyle);
}

/// @brief Draw a filled circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param color the color of the circle.
void Canvas::DrawBlockCircleFilled(int x,
                                   int y,
                                   int radius,
                                   const Color& color) noexcept {
  DrawBlockCircleFilled(x, y, radius,
                        [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a filled circle made of block characters.
/// @param x the x coordinate of the center of the circle.
/// @param y the y coordinate of the center of the circle.
/// @param radius the radius of the circle.
/// @param style the style of the circle.
void Canvas::DrawBlockCircleFilled(int x,
                                   int y,
                                   int radius,
                                   const Stylizer& s) noexcept {
  DrawBlockEllipseFilled(x, y, radius, radius, s);
}

/// @brief Draw an ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
void Canvas::DrawBlockEllipse(int x, int y, int r1, int r2) noexcept {
  DrawBlockEllipse(x, y, r1, r2, nostyle);
}

/// @brief Draw an ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param color the color of the ellipse.
void Canvas::DrawBlockEllipse(int x,
                              int y,
                              int r1,
                              int r2,
                              const Color& color) noexcept {
  DrawBlockEllipse(x, y, r1, r2,
                   [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw an ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param style the style of the ellipse.
void Canvas::DrawBlockEllipse(int x1,
                              int y1,
                              int r1,
                              int r2,
                              const Stylizer& s) noexcept {
  y1 /= 2;
  r2 /= 2;
  int x = -r1;
  int y = 0;
  int e2 = r2;
  int dx = (1 + 2 * x) * e2 * e2;
  int dy = x * x;
  int err = dx + dy;

  do {
    DrawBlock(x1 - x, 2 * (y1 + y), true, s);
    DrawBlock(x1 + x, 2 * (y1 + y), true, s);
    DrawBlock(x1 + x, 2 * (y1 - y), true, s);
    DrawBlock(x1 - x, 2 * (y1 - y), true, s);
    e2 = 2 * err;
    if (e2 >= dx) {
      x++;
      err += dx += 2 * r2 * r2;
    }
    if (e2 <= dy) {
      y++;
      err += dy += 2 * r1 * r1;
    }
  } while (x <= 0);

  while (y++ < r2) {
    DrawBlock(x1, 2 * (y1 + y), true, s);
    DrawBlock(x1, 2 * (y1 - y), true, s);
  }
}

/// @brief Draw a filled ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
void Canvas::DrawBlockEllipseFilled(int x, int y, int r1, int r2) noexcept {
  DrawBlockEllipseFilled(x, y, r1, r2, nostyle);
}

/// @brief Draw a filled ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param color the color of the ellipse.
void Canvas::DrawBlockEllipseFilled(int x,
                                    int y,
                                    int r1,
                                    int r2,
                                    const Color& color) noexcept {
  DrawBlockEllipseFilled(x, y, r1, r2,
                         [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a filled ellipse made of block characters.
/// @param x the x coordinate of the center of the ellipse.
/// @param y the y coordinate of the center of the ellipse.
/// @param r1 the radius of the ellipse along the x axis.
/// @param r2 the radius of the ellipse along the y axis.
/// @param style the style of the ellipse.
void Canvas::DrawBlockEllipseFilled(int x1,
                                    int y1,
                                    int r1,
                                    int r2,
                                    const Stylizer& s) noexcept {
  y1 /= 2;
  r2 /= 2;
  int x = -r1;
  int y = 0;
  int e2 = r2;
  int dx = (1 + 2 * x) * e2 * e2;
  int dy = x * x;
  int err = dx + dy;

  do {
    for (int xx = x1 + x; xx <= x1 - x; ++xx) {
      DrawBlock(xx, 2 * (y1 + y), true, s);
      DrawBlock(xx, 2 * (y1 - y), true, s);
    }
    e2 = 2 * err;
    if (e2 >= dx) {
      x++;
      err += dx += 2 * r2 * r2;
    }
    if (e2 <= dy) {
      y++;
      err += dy += 2 * r1 * r1;
    }
  } while (x <= 0);

  while (y++ < r2) {
    for (int yy = y1 + y; yy <= y1 - y; ++yy) {
      DrawBlock(x1, 2 * yy, true, s);
    }
  }
}

/// @brief Draw a piece of text.
/// @param x the x coordinate of the text.
/// @param y the y coordinate of the text.
/// @param value the text to draw.
void Canvas::DrawText(int x, int y, const std::string& value) noexcept {
  DrawText(x, y, value, nostyle);
}

/// @brief Draw a piece of text.
/// @param x the x coordinate of the text.
/// @param y the y coordinate of the text.
/// @param value the text to draw.
/// @param color the color of the text.
void Canvas::DrawText(int x,
                      int y,
                      const std::string& value,
                      const Color& color) noexcept {
  DrawText(x, y, value, [color](Pixel& p) { p.foreground_color = color; });
}

/// @brief Draw a piece of text.
/// @param x the x coordinate of the text.
/// @param y the y coordinate of the text.
/// @param value the text to draw.
/// @param style the style of the text.
void Canvas::DrawText(int x,
                      int y,
                      const std::string& value,
                      const Stylizer& style) noexcept {
  for (const auto& it : Utf8ToGlyphs(value)) {
    if (!IsIn(x, y)) {
      continue;
    }
    Cell& cell = storage_[XY{x / 2, y / 4}];
    cell.type = CellType::kText;
    cell.content.character = it;
    style(cell.content);
    x += 2;
  }
}

/// @brief Modify a pixel at a given location.
/// @param style a function that modifies the pixel.
void Canvas::Style(int x, int y, const Stylizer& style) noexcept {
  if (IsIn(x, y)) {
    style(storage_[XY{x / 2, y / 4}].content);
  }
}

namespace {

class CanvasNodeBase : public Node {
 public:
  CanvasNodeBase() = default;

  void Render(Screen& screen) noexcept override {
    const auto& c = canvas();
    const int y_max = ranges::min(c.height() / 4, box_.y_max - box_.y_min + 1);
    const int x_max = ranges::min(c.width() / 2, box_.x_max - box_.x_min + 1);
    for (int y = 0; y < y_max; ++y) {
      for (int x = 0; x < x_max; ++x) {
        screen.PixelAt(box_.x_min + x, box_.y_min + y) = c.GetPixel(x, y);
      }
    }
  }

  virtual const Canvas& canvas() = 0;
};

}  // namespace

/// @brief Produce an element from a Canvas, or a reference to a Canvas.
Element canvas(ConstRef<Canvas> canvas) noexcept {
  class Impl : public CanvasNodeBase {
   public:
    explicit Impl(ConstRef<Canvas> canvas) : canvas_(std::move(canvas)) {
      requirement_.min_x = (canvas_->width() + 1) / 2;
      requirement_.min_y = (canvas_->height() + 3) / 4;
    }
    const Canvas& canvas() noexcept final { return *canvas_; }
    ConstRef<Canvas> canvas_;
  };
  return std::make_shared<Impl>(std::move(canvas));
}

/// @brief Produce an element drawing a canvas of requested size.
/// @param width the width of the canvas.
/// @param height the height of the canvas.
/// @param fn a function drawing the canvas.
Element canvas(int width,
               int height,
               std::function<void(Canvas&)> fn) noexcept {
  class Impl : public CanvasNodeBase {
   public:
    Impl(int width, int height, std::function<void(Canvas&)> fn)
        : width_(width), height_(height), fn_(std::move(fn)) {}

    void ComputeRequirement() noexcept final {
      requirement_.min_x = (width_ + 1) / 2;
      requirement_.min_y = (height_ + 3) / 4;
    }

    void Render(Screen& screen) noexcept final {
      int width = (box_.y_max - box_.y_min + 1) * 2;
      int height = (box_.x_max - box_.x_min + 1) * 4;
      canvas_ = Canvas(width, height);
      fn_(canvas_);
      CanvasNodeBase::Render(screen);
    }

    const Canvas& canvas() noexcept final { return canvas_; }
    Canvas canvas_;
    int width_;
    int height_;
    std::function<void(Canvas&)> fn_;
  };
  return std::make_shared<Impl>(width, height, std::move(fn));
}

/// @brief Produce an element drawing a canvas.
/// @param fn a function drawing the canvas.
Element canvas(std::function<void(Canvas&)> fn) noexcept {
  static constexpr int default_dim = 12;
  return canvas(default_dim, default_dim, std::move(fn));
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
