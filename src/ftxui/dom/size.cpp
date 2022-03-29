#include <algorithm>  // for min, max
#include <cstddef>    // for size_t
#include <memory>     // for make_shared, __shared_ptr_access
#include <ranges>
#include <utility>  // for move
#include <vector>   // for __alloc_traits<>::value_type

#include <ftxui/dom/elements.hpp>  // for Constraint, Direction, EQUAL, GREATER_THAN, LESS_THAN, WIDTH, unpack, Decorator, Element, size
#include <ftxui/dom/node.hpp>      // for Node, Elements
#include <ftxui/dom/requirement.hpp>  // for Requirement
#include <ftxui/screen/box.hpp>       // for Box

namespace ranges = std::ranges;

namespace ftxui {

class Size : public Node {
 public:
  Size(Element child, Direction direction, Constraint constraint, size_t value)
      : Node(unpack(std::move(child))),
        value_(static_cast<int>(value)),
        direction_(direction),
        constraint_(constraint) {}

  void ComputeRequirement() noexcept override {
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();

    auto& value = direction_ == WIDTH ? requirement_.min_x : requirement_.min_y;

    switch (constraint_) {
      case LESS_THAN:
        value = ranges::min(value, value_);
        break;
      case EQUAL:
        value = value_;
        break;
      case GREATER_THAN:
        value = ranges::max(value, value_);
        break;
    }

    if (direction_ == WIDTH) {
      requirement_.flex_grow_x = 0;
      requirement_.flex_shrink_x = 0;
    } else {
      requirement_.flex_grow_y = 0;
      requirement_.flex_shrink_y = 0;
    }
  }

  void SetBox(Box box) noexcept override {
    Node::SetBox(box);

    if (direction_ == WIDTH) {
      switch (constraint_) {
        case LESS_THAN:
        case EQUAL:
          box.x_max = ranges::min(box.x_min + value_ + 1, box.x_max);
          break;
        case GREATER_THAN:
          break;
      }
    } else {
      switch (constraint_) {
        case LESS_THAN:
        case EQUAL:
          box.y_max = ranges::min(box.y_min + value_ + 1, box.y_max);
          break;
        case GREATER_THAN:
          break;
      }
    }
    children_[0]->SetBox(box);
  }

 private:
  int value_{};
  Direction direction_;
  Constraint constraint_;
};

/// @brief Apply a constraint on the size of an element.
/// @param direction Whether the WIDTH of the HEIGHT of the element must be
///                  constrained.
/// @param constraint The type of constraint.
/// @param value The value.
/// @ingroup dom
Decorator size(Direction direction, Constraint constraint, int value) noexcept {
  return [=](Element e) {
    return std::make_shared<Size>(std::move(e), direction, constraint, value);
  };
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
