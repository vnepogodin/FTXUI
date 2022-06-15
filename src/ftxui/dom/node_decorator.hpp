#ifndef FTXUI_DOM_NODE_DECORATOR_H_
#define FTXUI_DOM_NODE_DECORATOR_H_

#include <utility>  // for move

#include <ftxui/dom/elements.hpp>  // for Element, unpack
#include <ftxui/dom/node.hpp>      // for Node

namespace ftxui {
struct Box;

// Helper class.
class NodeDecorator : public Node {
 public:
  NodeDecorator(const Element& child) : Node(unpack(child)) {}
  void ComputeRequirement() noexcept override;
  void SetBox(const Box& box) noexcept override;
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_DOM_NODE_DECORATOR_H_ */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
