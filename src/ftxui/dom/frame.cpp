#include <algorithm>  // for max, min
#include <memory>     // for make_shared, __shared_ptr_access
#include <utility>    // for move
#include <vector>     // for __alloc_traits<>::value_type
#include <ranges>

#include <ftxui/dom/elements.hpp>  // for Element, unpack, Elements, focus, frame, select, xframe, yframe
#include <ftxui/dom/node.hpp>  // for Node, Elements
#include <ftxui/dom/requirement.hpp>  // for Requirement, Requirement::FOCUSED, Requirement::SELECTED
#include <ftxui/screen/box.hpp>      // for Box
#include <ftxui/screen/screen.hpp>   // for Screen, Screen::Cursor
#include <ftxui/util/autoreset.hpp>  // for AutoReset

namespace ranges = std::ranges;

namespace ftxui {

// -----------------------------------------------------------------------------

class Select : public Node {
 public:
  explicit Select(Elements children) : Node(std::move(children)) {}

  void ComputeRequirement() noexcept override {
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();
    auto& selected_box = requirement_.selected_box;
    selected_box.x_min = 0;
    selected_box.y_min = 0;
    selected_box.x_max = requirement_.min_x;
    selected_box.y_max = requirement_.min_y;
    requirement_.selection = Requirement::SELECTED;
  };

  void SetBox(Box box) noexcept override {
    Node::SetBox(box);
    children_[0]->SetBox(box);
  }
};

Element select(Element child) noexcept {
  return std::make_shared<Select>(unpack(std::move(child)));
}

// -----------------------------------------------------------------------------

class Focus : public Select {
 public:
  using Select::Select;

  void ComputeRequirement() noexcept override {
    Select::ComputeRequirement();
    requirement_.selection = Requirement::FOCUSED;
  };

  void Render(Screen& screen) noexcept override {
    Select::Render(screen);

    // Setting the cursor to the right position allow folks using CJK (China,
    // Japanese, Korean, ...) characters to see their [input method editor]
    // displayed at the right location. See [issue].
    //
    // [input method editor]:
    // https://en.wikipedia.org/wiki/Input_method
    //
    // [issue]:
    // https://github.com/ArthurSonzogni/FTXUI/issues/2#issuecomment-505282355
    //
    // Unfortunately, Microsoft terminal do not handle properly hidding the
    // cursor. Instead the character under the cursor is hidden, which is a big
    // problem. As a result, we can't enable setting cursor to the right
    // location. It will be displayed at the bottom right corner.
    // See:
    // https://github.com/microsoft/terminal/issues/1203
    // https://github.com/microsoft/terminal/issues/3093
#if !defined(FTXUI_MICROSOFT_TERMINAL_FALLBACK)
    screen.SetCursor(Screen::Cursor{box_.x_min, box_.y_min});
#endif
  }
};

Element focus(Element child) noexcept {
  return std::make_shared<Focus>(unpack(std::move(child)));
}

// -----------------------------------------------------------------------------

class Frame : public Node {
 public:
  Frame(Elements children, bool x_frame, bool y_frame)
      : Node(std::move(children)), x_frame_(x_frame), y_frame_(y_frame) {}

  void ComputeRequirement() noexcept override {
    Node::ComputeRequirement();
    requirement_ = children_[0]->requirement();
  }

  void SetBox(Box box) noexcept override {
    Node::SetBox(box);
    auto& selected_box = requirement_.selected_box;
    Box children_box = box;

    if (x_frame_) {
      const int external_dimx = box.x_max - box.x_min;
      const int internal_dimx = ranges::max(requirement_.min_x, external_dimx);
      const int focused_dimx = selected_box.x_max - selected_box.x_min;
      int dx = selected_box.x_min - external_dimx / 2 + focused_dimx / 2;
      dx = ranges::max(0, ranges::min(internal_dimx - external_dimx - 1, dx));
      children_box.x_min = box.x_min - dx;
      children_box.x_max = box.x_min + internal_dimx - dx;
    }

    if (y_frame_) {
      const int external_dimy = box.y_max - box.y_min;
      const int internal_dimy = ranges::max(requirement_.min_y, external_dimy);
      const int focused_dimy = selected_box.y_max - selected_box.y_min;
      int dy = selected_box.y_min - external_dimy / 2 + focused_dimy / 2;
      dy = ranges::max(0, ranges::min(internal_dimy - external_dimy - 1, dy));
      children_box.y_min = box.y_min - dy;
      children_box.y_max = box.y_min + internal_dimy - dy;
    }

    children_[0]->SetBox(children_box);
  }

  void Render(Screen& screen) noexcept override {
    AutoReset<Box> stencil(&screen.stencil,
                           Box::Intersection(box_, screen.stencil));
    children_[0]->Render(screen);
  }

 private:
  bool x_frame_{};
  bool y_frame_{};
};

/// @brief Allow an element to be displayed inside a 'virtual' area. It size can
/// be larger than its container. In this case only a smaller portion is
/// displayed. The view is scrollable to make the focused element visible.
/// @see focus
Element frame(Element child) noexcept {
  return std::make_shared<Frame>(unpack(std::move(child)), true, true);
}

Element xframe(Element child) noexcept {
  return std::make_shared<Frame>(unpack(std::move(child)), true, false);
}

Element yframe(Element child) noexcept {
  return std::make_shared<Frame>(unpack(std::move(child)), false, true);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
