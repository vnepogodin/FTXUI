#ifndef FTXUI_DOM_NODE_HPP
#define FTXUI_DOM_NODE_HPP

#include <memory>  // for shared_ptr
#include <vector>  // for vector

#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box
#include "ftxui/screen/screen.hpp"

namespace ftxui {

class Node;
class Screen;

using Element = std::shared_ptr<Node>;
using Elements = std::vector<Element>;

class Node {
 public:
  constexpr Node() noexcept = default;
  Node(Elements children);
  virtual ~Node() = default;

  // Step 1: Compute layout requirement. Tell parent what dimensions this
  //         element wants to be.
  //         Propagated from Children to Parents.
  virtual void ComputeRequirement();
  constexpr Requirement requirement() { return requirement_; }

  // Step 2: Assign this element its final dimensions.
  //         Propagated from Parents to Children.
  virtual void SetBox(Box box);

  // Step 3: Draw this element.
  virtual void Render(Screen& screen);

  // Layout may not resolve within a single iteration for some elements. This
  // allows them to request additionnal iterations. This signal must be
  // forwarded to children at least once.
  struct Status {
    int iteration = 0;
    bool need_iteration = false;
  };
  virtual void Check(Status* status);

 protected:
  Elements children_{};
  Requirement requirement_{};
  Box box_{};
};

/// @brief Display an element on a ftxui::Screen.
/// @ingroup dom
constexpr void Render(Screen& screen, Node* node) noexcept {
  const auto box = Box{0, screen.dimx() - 1, 0, screen.dimy() - 1};
  Node::Status status;
  node->Check(&status);
  while (status.need_iteration && status.iteration < 20) {
    // Step 1: Find what dimension this elements wants to be.
    node->ComputeRequirement();

    // Step 2: Assign a dimension to the element.
    node->SetBox(box);

    // Check if the element needs another iteration of the layout algorithm.
    status.need_iteration = false;
    status.iteration++;
    node->Check(&status);
  }

  // Step 3: Draw the element.
  screen.stencil = box;
  node->Render(screen);

  // Step 4: Apply shaders
  screen.ApplyShader();
}

/// @brief Display an element on a ftxui::Screen.
/// @ingroup dom
constexpr void Render(Screen& screen, const Element& element) noexcept {
  Render(screen, element.get());
}

}  // namespace ftxui

#endif /* end of include guard: FTXUI_DOM_NODE_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
