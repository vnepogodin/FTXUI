#include <memory>     // for __shared_ptr_access, shared_ptr, make_unique
#include <utility>  // for move
#include <vector>   // for vector

#include <ftxui/dom/elements.hpp>     // for Element, Elements, dbox
#include <ftxui/dom/node.hpp>         // for Node
#include <ftxui/dom/requirement.hpp>  // for Requirement
#include <ftxui/screen/box.hpp>       // for Box

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <range/v3/algorithm/max.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace ftxui {

class DBox : public Node {
 public:
  explicit DBox(const Elements& children) : Node(children) {}

  void ComputeRequirement() noexcept override {
    requirement_.min_x = 0;
    requirement_.min_y = 0;
    requirement_.flex_grow_x = 0;
    requirement_.flex_grow_y = 0;
    requirement_.flex_shrink_x = 0;
    requirement_.flex_shrink_y = 0;
    requirement_.selection = Requirement::NORMAL;
    for (auto& child : children_) {
      child->ComputeRequirement();
      requirement_.min_x =
          ranges::max(requirement_.min_x, child->requirement().min_x);
      requirement_.min_y =
          ranges::max(requirement_.min_y, child->requirement().min_y);

      if (requirement_.selection < child->requirement().selection) {
        requirement_.selection = child->requirement().selection;
        requirement_.selected_box = child->requirement().selected_box;
      }
    }
  }

  void SetBox(const Box& box) noexcept override {
    Node::SetBox(box);

    for (auto& child : children_) {
      child->SetBox(box);
    }
  }
};

/// @brief Stack several element on top of each other.
/// @param children_ The input element.
/// @return The right aligned element.
/// @ingroup dom
Element dbox(const Elements& children_) noexcept {
  return std::make_unique<DBox>(children_);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
