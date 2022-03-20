#ifndef FTXUI_DOM_FLEXBOX_HELPER_HPP
#define FTXUI_DOM_FLEXBOX_HELPER_HPP

#include <vector>
#include <ftxui/dom/flexbox_config.hpp>

namespace ftxui::flexbox_helper {

struct Block {
  // Input:
  int min_size_x{};
  int min_size_y{};
  int flex_grow_x{};
  int flex_grow_y{};
  int flex_shrink_x{};
  int flex_shrink_y{};

  // Output:
  int line{};
  int line_position{};
  int x{};
  int y{};
  int dim_x{};
  int dim_y{};
  bool overflow{false};
};

struct Global {
  int size_x{};
  int size_y{};

  std::vector<Block> blocks;
  FlexboxConfig config;
};

void Compute(Global& global) noexcept;

}  // namespace ftxui

#endif /* end of include guard: FTXUI_DOM_FLEXBOX_HELPER_HPP*/

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
