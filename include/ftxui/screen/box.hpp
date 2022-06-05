#ifndef FTXUI_SCREEN_BOX_HPP
#define FTXUI_SCREEN_BOX_HPP

#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/min.hpp>

namespace ftxui {

struct Box {
  int x_min = 0;
  int x_max = 0;
  int y_min = 0;
  int y_max = 0;

  /// @return the biggest Box contained in both |a| and |b|.
  /// @ingroup screen
  // static
  static constexpr inline Box Intersection(Box a, Box b) noexcept {
    return Box{
        ranges::max(a.x_min, b.x_min),
        ranges::min(a.x_max, b.x_max),
        ranges::max(a.y_min, b.y_min),
        ranges::min(a.y_max, b.y_max),
    };
  }
  static constexpr inline Box Union(Box a, Box b) noexcept {
    return Box{
        ranges::min(a.x_min, b.x_min),
        ranges::max(a.x_max, b.x_max),
        ranges::min(a.y_min, b.y_min),
        ranges::max(a.y_max, b.y_max),
    };
  }

  /// @return whether (x,y) is contained inside the box.
  /// @ingroup screen
  [[nodiscard]] constexpr inline bool Contain(int x, int y) const noexcept {
    return x_min <= x &&  //
           x_max >= x &&  //
           y_min <= y &&  //
           y_max >= y;
  }

  /// @return whether |other| is the same as |this|
  /// @ingroup screen
  constexpr inline bool operator==(const Box& other) const noexcept {
    return (x_min == other.x_min) && (x_max == other.x_max) &&
           (y_min == other.y_min) && (y_max == other.y_max);
  }

  /// @return whether |other| and |this| are different.
  /// @ingroup screen
  constexpr inline bool operator!=(const Box& other) const noexcept {
    return !operator==(other);
  }
};

}  // namespace ftxui

#endif  // FTXUI_SCREEN_BOX_HPP

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
