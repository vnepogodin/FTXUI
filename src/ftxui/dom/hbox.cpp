#include <cstddef>    // for size_t
#include <memory>  // for __shared_ptr_access, shared_ptr, make_unique, allocator_traits<>::value_type
#include <utility>  // for move
#include <vector>   // for vector, __alloc_traits<>::value_type

#include <ftxui/dom/box_helper.hpp>   // for Element, Compute
#include <ftxui/dom/elements.hpp>     // for Element, Elements, hbox
#include <ftxui/dom/node.hpp>         // for Node, Elements
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

class HBox : public Node {
 public:
  explicit HBox(const Elements& children) : Node(children) {}

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
      if (requirement_.selection < child->requirement().selection) {
        requirement_.selection = child->requirement().selection;
        requirement_.selected_box = child->requirement().selected_box;
        requirement_.selected_box.x_min += requirement_.min_x;
        requirement_.selected_box.x_max += requirement_.min_x;
      }
      requirement_.min_x += child->requirement().min_x;
      requirement_.min_y =
          ranges::max(requirement_.min_y, child->requirement().min_y);
    }
  }

  void SetBox(const Box& box) noexcept override {
    Node::SetBox(box);

    std::vector<box_helper::Element> elements(children_.size());
    for (std::size_t i = 0; i < children_.size(); ++i) {
      auto& element = elements[i];
      const auto& requirement = children_[i]->requirement();
      element.min_size = requirement.min_x;
      element.flex_grow = requirement.flex_grow_x;
      element.flex_shrink = requirement.flex_shrink_x;
    }
    const int target_size = box.x_max - box.x_min + 1;
    box_helper::Compute(&elements, target_size);

    Box temp = box;
    int x = box.x_min;
    for (std::size_t i = 0; i < children_.size(); ++i) {
      temp.x_min = x;
      temp.x_max = x + elements[i].size - 1;
      children_[i]->SetBox(temp);
      x = temp.x_max + 1;
    }
  }
};

/// @brief A container displaying elements horizontally one by one.
/// @param children The elements in the container
/// @return The container.
///
/// #### Example
///
/// ```cpp
/// hbox({
///   text("Left"),
///   text("Right"),
/// });
/// ```
Element hbox(const Elements& children) noexcept {
  return std::make_unique<HBox>(children);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
