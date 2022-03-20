#ifndef FTXUI_DOM_TABLE
#define FTXUI_DOM_TABLE

#include <memory>
#include <string>  // for string
#include <vector>  // for vector

#include <ftxui/dom/elements.hpp>  // for Element, BorderStyle, LIGHT, Decorator

namespace ftxui {

// Usage:
//
// Initialization:
// ---------------
//
// auto table = Table({
//   {"X", "Y"},
//   {"-1", "1"},
//   {"+0", "0"},
//   {"+1", "1"},
// });
//
// table.SelectAll().Border(LIGHT);
//
// table.SelectRow(1).Border(DOUBLE);
// table.SelectRow(1).SeparatorInternal(Light);
//
// std::move(table).Element();

class Table;
class TableSelection;

class Table {
 public:
  Table() noexcept;
  Table(const std::vector<std::vector<std::string>>&) noexcept;
  Table(const std::vector<std::vector<Element>>&) noexcept;
  TableSelection SelectAll() noexcept;
  TableSelection SelectCell(int column, int row) noexcept;
  TableSelection SelectRow(int row_index) noexcept;
  TableSelection SelectRows(int row_min, int row_max) noexcept;
  TableSelection SelectColumn(int column_index) noexcept;
  TableSelection SelectColumns(int column_min, int column_max) noexcept;
  TableSelection SelectRectangle(int column_min,
                                 int column_max,
                                 int row_min,
                                 int row_max) noexcept;
  Element Render() noexcept;

 private:
  void Initialize(std::vector<std::vector<Element>>) noexcept;
  friend TableSelection;

  int input_dim_x_{};
  int input_dim_y_{};
  int dim_x_{};
  int dim_y_{};

  std::vector<std::vector<Element>> elements_;
};

class TableSelection {
 public:
  void Decorate(const Decorator&) noexcept;
  void DecorateAlternateRow(const Decorator&, int modulo = 2, int shift = 0) noexcept;
  void DecorateAlternateColumn(const Decorator&, int modulo = 2, int shift = 0) noexcept;

  void DecorateCells(const Decorator&) noexcept;
  void DecorateCellsAlternateColumn(const Decorator&, int modulo = 2, int shift = 0) noexcept;
  void DecorateCellsAlternateRow(const Decorator&, int modulo = 2, int shift = 0) noexcept;

  void Border(BorderStyle border = LIGHT) noexcept;
  void BorderLeft(BorderStyle border = LIGHT) noexcept;
  void BorderRight(BorderStyle border = LIGHT) noexcept;
  void BorderTop(BorderStyle border = LIGHT) noexcept;
  void BorderBottom(BorderStyle border = LIGHT) noexcept;

  void Separator(BorderStyle border = LIGHT) noexcept;
  void SeparatorVertical(BorderStyle border = LIGHT) noexcept;
  void SeparatorHorizontal(BorderStyle border = LIGHT) noexcept;

 private:
  friend Table;

  int x_min_;
  int x_max_;
  int y_min_;
  int y_max_;

  Table* table_;
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_DOM_TABLE */

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
