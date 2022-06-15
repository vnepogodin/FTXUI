#include <memory>   // for make_unique, __shared_ptr_access
#include <utility>  // for move
#include <vector>   // for __alloc_traits<>::value_type

#include "ftxui/dom/elements.hpp"     // for Element, unpack, Decorator, reflect
#include "ftxui/dom/node.hpp"         // for Node, Elements
#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box
#include "ftxui/screen/screen.hpp"    // for Screen

namespace ftxui {

// Helper class.
class Reflect : public Node {
 public:
  Reflect(const Element& child, Box& box)
      : Node(unpack(child)), reflected_box_(box) {}

  void ComputeRequirement() noexcept final {
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();
  }

  void SetBox(const Box& box) noexcept final {
    reflected_box_ = box;
    Node::SetBox(box);
    children_[0]->SetBox(box);
  }

  void Render(Screen& screen) noexcept final {
    reflected_box_ = Box::Intersection(screen.stencil, reflected_box_);
    return Node::Render(screen);
  }

 private:
  Box& reflected_box_;
};

[[gnu::const]]
Decorator reflect(Box& box) noexcept {
  return [&](auto&& child) -> Element {
    return std::make_unique<Reflect>(child, box);
  };
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
