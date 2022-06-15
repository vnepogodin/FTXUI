#include <cstddef>    // for size_t
#include <memory>  // for __shared_ptr_access, shared_ptr, make_unique, allocator_traits<>::value_type
#include <vector>   // for vector, __alloc_traits<>::value_type

#include <ftxui/dom/box_helper.hpp>   // for Element, Compute
#include <ftxui/dom/elements.hpp>     // for Element, Elements, vbox
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

class VBox : public Node {
 public:
  explicit VBox(const Elements& children) : Node(children) {}

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
        requirement_.selected_box.y_min += requirement_.min_y;
        requirement_.selected_box.y_max += requirement_.min_y;
      }
      requirement_.min_y += child->requirement().min_y;
      requirement_.min_x =
          ranges::max(requirement_.min_x, child->requirement().min_x);
    }
  }

  void SetBox(const Box& box) noexcept override {
    Node::SetBox(box);

    std::vector<box_helper::Element> elements(children_.size());
    for (size_t i = 0; i < children_.size(); ++i) {
      auto& element = elements[i];
      const auto& requirement = children_[i]->requirement();
      element.min_size = requirement.min_y;
      element.flex_grow = requirement.flex_grow_y;
      element.flex_shrink = requirement.flex_shrink_y;
    }
    const int target_size = box.y_max - box.y_min + 1;
    box_helper::Compute(&elements, target_size);

    Box temp = box;
    int y = box.y_min;
    for (size_t i = 0; i < children_.size(); ++i) {
      temp.y_min = y;
      temp.y_max = y + elements[i].size - 1;
      children_[i]->SetBox(temp);
      y = temp.y_max + 1;
    }
  }
};

/// @brief A container displaying elements vertically one by one.
/// @param children The elements in the container
/// @return The container.
/// @ingroup dom
///
/// #### Example
///
/// ```cpp
/// vbox({
///   text("Up"),
///   text("Down"),
/// });
/// ```
Element vbox(const Elements& children) noexcept {
  return std::make_unique<VBox>(children);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
