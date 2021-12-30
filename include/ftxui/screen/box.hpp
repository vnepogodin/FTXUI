#ifndef FTXUI_SCREEN_BOX_HPP
#define FTXUI_SCREEN_BOX_HPP

#include <algorithm>

namespace ftxui {

struct Box {
  int x_min = 0;
  int x_max = 0;
  int y_min = 0;
  int y_max = 0;

  /// @return the biggest Box contained in both |a| and |b|.
  /// @ingroup screen
  // static
  static constexpr Box Intersection(Box a, Box b) noexcept {
    return Box{
        std::max(a.x_min, b.x_min),
        std::min(a.x_max, b.x_max),
        std::max(a.y_min, b.y_min),
        std::min(a.y_max, b.y_max),
    };
  }

  /// @return whether (x,y) is contained inside the box.
  /// @ingroup screen
  constexpr bool Contain(int x, int y) const noexcept {
    return x_min <= x &&  //
           x_max >= x &&  //
           y_min <= y &&  //
           y_max >= y;
  }

  /// @return whether |other| is the same as |this|
  /// @ingroup screen
  constexpr bool operator==(const Box& other) const noexcept {
    return (x_min == other.x_min) && (x_max == other.x_max) &&
           (y_min == other.y_min) && (y_max == other.y_max);
  }

  /// @return whether |other| and |this| are different.
  /// @ingroup screen
  constexpr bool operator!=(const Box& other) const noexcept {
    return !operator==(other);
  }
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_SCREEN_BOX_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
