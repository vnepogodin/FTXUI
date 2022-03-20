#ifndef FTXUI_DOM_BOX_HELPER_HPP
#define FTXUI_DOM_BOX_HELPER_HPP

#include <vector>

namespace ftxui::box_helper {

struct Element {
  // Input:
  int min_size{};
  int flex_grow{};
  int flex_shrink{};

  // Output;
  int size{};
};

void Compute(std::vector<Element>* elements, int target_size) noexcept;

}  // namespace ftxui

#endif /* end of include guard: FTXUI_DOM_BOX_HELPER_HPP */

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.line.
