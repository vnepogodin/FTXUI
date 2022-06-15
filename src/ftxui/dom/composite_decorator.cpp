#include <ftxui/dom/elements.hpp>  // for Element, filler, operator|, hbox, flex_grow, vbox, xflex_grow, yflex_grow, align_right, center, hcenter, vcenter

namespace ftxui {

/// @brief Center an element horizontally.
/// @param child The decorated element.
/// @return The centered element.
/// @ingroup dom
Element hcenter(const Element& child) noexcept {
  return hbox(filler(), child, filler());
}

/// @brief Center an element vertically.
/// @param child The decorated element.
/// @return The centered element.
/// @ingroup dom
Element vcenter(const Element& child) noexcept {
  return vbox(filler(), child, filler());
}

/// @brief Center an element horizontally and vertically.
/// @param child The decorated element.
/// @return The centered element.
/// @ingroup dom
Element center(const Element& child) noexcept {
  return hcenter(vcenter(child));
}

/// @brief Align an element on the right side.
/// @param child The decorated element.
/// @return The right aligned element.
/// @ingroup dom
Element align_right(const Element& child) noexcept {
  return hbox(filler(), child);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
