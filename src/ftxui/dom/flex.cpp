#include <memory>   // for make_unique, __shared_ptr_access
#include <vector>   // for __alloc_traits<>::value_type

#include "ftxui/dom/elements.hpp"  // for Element, unpack, filler, flex, flex_grow, flex_shrink, notflex, xflex, xflex_grow, xflex_shrink, yflex, yflex_grow, yflex_shrink
#include "ftxui/dom/node.hpp"      // for Elements, Node
#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box

namespace ftxui {

namespace {

using FlexFunction = void (*)(Requirement&);

constexpr void function_flex_grow(Requirement& r) noexcept {
  r.flex_grow_x = 1;
  r.flex_grow_y = 1;
}

constexpr void function_xflex_grow(Requirement& r) noexcept {
  r.flex_grow_x = 1;
}

constexpr void function_yflex_grow(Requirement& r) noexcept {
  r.flex_grow_y = 1;
}

constexpr void function_flex_shrink(Requirement& r) noexcept {
  r.flex_shrink_x = 1;
  r.flex_shrink_y = 1;
}

constexpr void function_xflex_shrink(Requirement& r) noexcept {
  r.flex_shrink_x = 1;
}

constexpr void function_yflex_shrink(Requirement& r) noexcept {
  r.flex_shrink_y = 1;
}

constexpr void function_flex(Requirement& r) noexcept {
  r.flex_grow_x = 1;
  r.flex_grow_y = 1;
  r.flex_shrink_x = 1;
  r.flex_shrink_y = 1;
}

constexpr void function_xflex(Requirement& r) noexcept {
  r.flex_grow_x = 1;
  r.flex_shrink_x = 1;
}

constexpr void function_yflex(Requirement& r) noexcept {
  r.flex_grow_y = 1;
  r.flex_shrink_y = 1;
}

constexpr void function_not_flex(Requirement& r) noexcept {
  r.flex_grow_x = 0;
  r.flex_grow_y = 0;
  r.flex_shrink_x = 0;
  r.flex_shrink_y = 0;
}

}  // namespace

class Flex : public Node {
 public:
  explicit Flex(const FlexFunction& f) : f_(f) {}
  Flex(const FlexFunction& f, const Element& child) : Node(unpack(child)), f_(f) {}
  void ComputeRequirement() noexcept override {
    requirement_.min_x = 0;
    requirement_.min_y = 0;
    if (!children_.empty()) {
      children_[0]->ComputeRequirement();
      requirement_ = children_[0]->requirement();
    }
    f_(requirement_);
  }

  void SetBox(const Box& box) noexcept override {
    if (children_.empty()) {
      return;
    }
    children_[0]->SetBox(box);
  }

  FlexFunction f_;
};

/// @brief An element that will take expand proportionally to the space left in
/// a container.
/// @ingroup dom
Element filler() noexcept {
  return std::make_unique<Flex>(function_flex);
}

/// @brief Make a child element to expand proportionally to the space left in a
/// container.
/// @ingroup dom
///
/// #### Examples:
///
/// ~~~cpp
///   hbox({
///     text("left") | border ,
///     text("middle") | border | flex,
///     text("right") | border,
///   });
/// ~~~
///
/// #### Output:
///
/// ~~~bash
/// ┌────┐┌─────────────────────────────────────────────────────────┐┌─────┐
/// │left││middle                                                   ││right│
/// └────┘└─────────────────────────────────────────────────────────┘└─────┘
/// ~~~
Element flex(const Element& child) noexcept {
  return std::make_unique<Flex>(function_flex, child);
}

/// @brief Expand/Minimize if possible/needed on the X axis.
/// @ingroup dom
Element xflex(const Element& child) noexcept {
  return std::make_unique<Flex>(function_xflex, child);
}

/// @brief Expand/Minimize if possible/needed on the Y axis.
/// @ingroup dom
Element yflex(const Element& child) noexcept {
  return std::make_unique<Flex>(function_yflex, child);
}

/// @brief Expand if possible.
/// @ingroup dom
Element flex_grow(const Element& child) noexcept {
  return std::make_unique<Flex>(function_flex_grow, child);
}

/// @brief Expand if possible on the X axis.
/// @ingroup dom
Element xflex_grow(const Element& child) noexcept {
  return std::make_unique<Flex>(function_xflex_grow, child);
}

/// @brief Expand if possible on the Y axis.
/// @ingroup dom
Element yflex_grow(const Element& child) noexcept {
  return std::make_unique<Flex>(function_yflex_grow, child);
}

/// @brief Minimize if needed.
/// @ingroup dom
Element flex_shrink(const Element& child) noexcept {
  return std::make_unique<Flex>(function_flex_shrink, child);
}

/// @brief Minimize if needed on the X axis.
/// @ingroup dom
Element xflex_shrink(const Element& child) noexcept {
  return std::make_unique<Flex>(function_xflex_shrink, child);
}

/// @brief Minimize if needed on the Y axis.
/// @ingroup dom
Element yflex_shrink(const Element& child) noexcept {
  return std::make_unique<Flex>(function_yflex_shrink, child);
}

/// @brief Make the element not flexible.
/// @ingroup dom
Element notflex(const Element& child) noexcept {
  return std::make_unique<Flex>(function_not_flex, child);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
