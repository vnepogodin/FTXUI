#include "ftxui/dom/table.hpp"

#include <algorithm>  // for max
#include <memory>  // for allocator, shared_ptr, allocator_traits<>::value_type
#include <ranges>
#include <utility>  // for move, swap

#include <ftxui/dom/elements.hpp>  // for Element, operator|, text, separatorCharacter, Elements, BorderStyle, Decorator, emptyElement, size, gridbox, EQUAL, flex, flex_shrink, HEIGHT, WIDTH

namespace ranges = std::ranges;

namespace ftxui {
namespace {

// NOLINTNEXTLINE
static const std::string charset[6][6] = {
    {"┌", "┐", "└", "┘", "─", "│"},  //
    {"┏", "┓", "┗", "┛", "━", "┃"},  //
    {"╔", "╗", "╚", "╝", "═", "║"},  //
    {"╭", "╮", "╰", "╯", "─", "│"},  //
    {" ", " ", " ", " ", " ", " "},  //
};

constexpr bool IsCell(int x, int y) noexcept {
  return x % 2 == 1 && y % 2 == 1;
}

constexpr int Wrap(int input, int modulo) noexcept {
  input %= modulo;
  input += modulo;
  input %= modulo;
  return input;
}

constexpr void Order(int& a, int& b) noexcept {
  if (a >= b) {
    std::swap(a, b);
  }
}

}  // namespace

Table::Table() noexcept {
  Initialize({});
}

Table::Table(const std::vector<std::vector<std::string>>& input) noexcept {
  std::vector<std::vector<Element>> output;
  for (auto& row : input) {
    output.emplace_back();
    auto& output_row = output.back();
    for (auto& cell : row) {
      output_row.push_back(text(std::move(cell)));
    }
  }
  Initialize(std::move(output));
}

Table::Table(const std::vector<std::vector<Element>>& input) noexcept {
  Initialize(input);
}

void Table::Initialize(std::vector<std::vector<Element>> input) noexcept {
  input_dim_y_ = static_cast<int>(input.size());
  input_dim_x_ = 0;
  for (auto&& row : input)
    input_dim_x_ = ranges::max(input_dim_x_, static_cast<int>(row.size()));

  dim_y_ = 2 * input_dim_y_ + 1;
  dim_x_ = 2 * input_dim_x_ + 1;

  // Reserve space.
  elements_.resize(dim_y_);
  for (int y = 0; y < dim_y_; ++y) {
    elements_[y].resize(dim_x_);
  }

  // Transfert elements_ from |input| toward |elements_|.
  {
    int y = 1;
    for (auto& row : input) {
      int x = 1;
      for (auto& cell : row) {
        elements_[y][x] = std::move(cell);
        x += 2;
      }
      y += 2;
    }
  }

  // Add empty element for the border.
  for (int y = 0; y < dim_y_; ++y) {
    for (int x = 0; x < dim_x_; ++x) {
      auto& element = elements_[y][x];

      if (IsCell(x, y)) {
        if (!element) {
          element = emptyElement();
        }
        continue;
      }

      element = emptyElement();
    }
  }
}

[[gnu::pure]] TableSelection Table::SelectRow(int index) noexcept {
  return SelectRectangle(0, -1, index, index);
}

[[gnu::pure]] TableSelection Table::SelectRows(int row_min, int row_max) noexcept {
  return SelectRectangle(0, -1, row_min, row_max);
}

[[gnu::pure]] TableSelection Table::SelectColumn(int index) noexcept {
  return SelectRectangle(index, index, 0, -1);
}

[[gnu::pure]] TableSelection Table::SelectColumns(int column_min, int column_max) noexcept {
  return SelectRectangle(column_min, column_max, 0, -1);
}

[[gnu::pure]] TableSelection Table::SelectCell(int column, int row) noexcept {
  return SelectRectangle(column, column, row, row);
}

[[gnu::pure]] TableSelection Table::SelectRectangle(int column_min,
                                      int column_max,
                                      int row_min,
                                      int row_max) noexcept {
  column_min = Wrap(column_min, input_dim_x_);
  column_max = Wrap(column_max, input_dim_x_);
  Order(column_min, column_max);
  row_min = Wrap(row_min, input_dim_y_);
  row_max = Wrap(row_max, input_dim_y_);
  Order(row_min, row_max);

  TableSelection output{};  // NOLINT
  output.table_ = this;
  output.x_min_ = 2 * column_min;
  output.x_max_ = 2 * column_max + 2;
  output.y_min_ = 2 * row_min;
  output.y_max_ = 2 * row_max + 2;
  return output;
}

[[gnu::pure]] TableSelection Table::SelectAll() noexcept {
  TableSelection output{};  // NOLINT
  output.table_ = this;
  output.x_min_ = 0;
  output.x_max_ = dim_x_ - 1;
  output.y_min_ = 0;
  output.y_max_ = dim_y_ - 1;
  return output;
}

Element Table::Render() noexcept {
  for (int y = 0; y < dim_y_; ++y) {
    for (int x = 0; x < dim_x_; ++x) {
      auto& it = elements_[y][x];

      // Line
      if ((x + y) % 2 == 1) {
        it = std::move(it) | flex;
        continue;
      }

      // Cells
      if ((x % 2) == 1 && (y % 2) == 1) {
        it = std::move(it) | flex_shrink;
        continue;
      }

      // Corners
      it = std::move(it) | size(WIDTH, EQUAL, 0) | size(HEIGHT, EQUAL, 0);
    }
  }
  dim_x_ = 0;
  dim_y_ = 0;
  return gridbox(std::move(elements_));
}

// NOLINTNEXTLINE
void TableSelection::Decorate(const Decorator& decorator) noexcept {
  for (int y = y_min_; y <= y_max_; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      Element& e = table_->elements_[y][x];
      e = std::move(e) | decorator;
    }
  }
}

// NOLINTNEXTLINE
void TableSelection::DecorateCells(const Decorator& decorator) noexcept {
  for (int y = y_min_; y <= y_max_; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      if (y % 2 == 1 && x % 2 == 1) {
        Element& e = table_->elements_[y][x];
        e = std::move(e) | decorator;
      }
    }
  }
}

// NOLINTNEXTLINE
void TableSelection::DecorateAlternateColumn(const Decorator& decorator,
                                             int modulo,
                                             int shift) noexcept {
  for (int y = y_min_; y <= y_max_; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      if (y % 2 == 1 && (x / 2) % modulo == shift) {
        Element& e = table_->elements_[y][x];
        e = std::move(e) | decorator;
      }
    }
  }
}

// NOLINTNEXTLINE
void TableSelection::DecorateAlternateRow(const Decorator& decorator,
                                          int modulo,
                                          int shift) noexcept {
  for (int y = y_min_ + 1; y <= y_max_ - 1; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      if (y % 2 == 1 && (y / 2) % modulo == shift) {
        Element& e = table_->elements_[y][x];
        e = std::move(e) | decorator;
      }
    }
  }
}

// NOLINTNEXTLINE
void TableSelection::DecorateCellsAlternateColumn(const Decorator& decorator,
                                                  int modulo,
                                                  int shift) noexcept {
  for (int y = y_min_; y <= y_max_; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      if (y % 2 == 1 && x % 2 == 1 && ((x / 2) % modulo == shift)) {
        Element& e = table_->elements_[y][x];
        e = std::move(e) | decorator;
      }
    }
  }
}

// NOLINTNEXTLINE
void TableSelection::DecorateCellsAlternateRow(const Decorator& decorator,
                                               int modulo,
                                               int shift) noexcept {
  for (int y = y_min_; y <= y_max_; ++y) {
    for (int x = x_min_; x <= x_max_; ++x) {
      if (y % 2 == 1 && x % 2 == 1 && ((y / 2) % modulo == shift)) {
        Element& e = table_->elements_[y][x];
        e = std::move(e) | decorator;
      }
    }
  }
}

void TableSelection::Border(BorderStyle border) noexcept {
  BorderLeft(border);
  BorderRight(border);
  BorderTop(border);
  BorderBottom(border);

  // NOLINTNEXTLINE
  table_->elements_[y_min_][x_min_] = text(charset[border][0]) | automerge;
  // NOLINTNEXTLINE
  table_->elements_[y_min_][x_max_] = text(charset[border][1]) | automerge;
  // NOLINTNEXTLINE
  table_->elements_[y_max_][x_min_] = text(charset[border][2]) | automerge;
  // NOLINTNEXTLINE
  table_->elements_[y_max_][x_max_] = text(charset[border][3]) | automerge;
}

void TableSelection::Separator(BorderStyle border) noexcept {
  for (int y = y_min_ + 1; y <= y_max_ - 1; ++y) {
    for (int x = x_min_ + 1; x <= x_max_ - 1; ++x) {
      if (y % 2 == 0 || x % 2 == 0) {
        Element& e = table_->elements_[y][x];
        e = (y % 2 == 1)
                ? separatorCharacter(charset[border][5]) | automerge   // NOLINT
                : separatorCharacter(charset[border][4]) | automerge;  // NOLINT
      }
    }
  }
}

void TableSelection::SeparatorVertical(BorderStyle border) noexcept {
  for (int y = y_min_ + 1; y <= y_max_ - 1; ++y) {
    for (int x = x_min_ + 1; x <= x_max_ - 1; ++x) {
      if (x % 2 == 0) {
        table_->elements_[y][x] =
            separatorCharacter(charset[border][5]) | automerge;  // NOLINT
      }
    }
  }
}

void TableSelection::SeparatorHorizontal(BorderStyle border) noexcept {
  for (int y = y_min_ + 1; y <= y_max_ - 1; ++y) {
    for (int x = x_min_ + 1; x <= x_max_ - 1; ++x) {
      if (y % 2 == 0) {
        table_->elements_[y][x] =
            separatorCharacter(charset[border][4]) | automerge;  // NOLINT
      }
    }
  }
}

void TableSelection::BorderLeft(BorderStyle border) noexcept {
  for (int y = y_min_; y <= y_max_; y++) {
    table_->elements_[y][x_min_] =
        separatorCharacter(charset[border][5]) | automerge;  // NOLINT
  }
}

void TableSelection::BorderRight(BorderStyle border) noexcept {
  for (int y = y_min_; y <= y_max_; y++) {
    table_->elements_[y][x_max_] =
        separatorCharacter(charset[border][5]) | automerge;  // NOLINT
  }
}

void TableSelection::BorderTop(BorderStyle border) noexcept {
  for (int x = x_min_; x <= x_max_; x++) {
    table_->elements_[y_min_][x] =
        separatorCharacter(charset[border][4]) | automerge;  // NOLINT
  }
}

void TableSelection::BorderBottom(BorderStyle border) noexcept {
  for (int x = x_min_; x <= x_max_; x++) {
    table_->elements_[y_max_][x] =
        separatorCharacter(charset[border][4]) | automerge;  // NOLINT
  }
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
