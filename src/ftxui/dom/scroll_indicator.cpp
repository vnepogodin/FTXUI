#include <memory>     // for make_unique, __shared_ptr_access
#include <string>   // for string
#include <vector>   // for __alloc_traits<>::value_type

#include <ftxui/dom/elements.hpp>        // for Element, vscroll_indicator
#include <ftxui/dom/node.hpp>            // for Node, Elements
#include <ftxui/dom/node_decorator.hpp>  // for NodeDecorator
#include <ftxui/dom/requirement.hpp>     // for Requirement
#include <ftxui/screen/box.hpp>          // for Box
#include <ftxui/screen/screen.hpp>       // for Screen, Pixel

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

/// @brief Add a filter that will invert the foreground and the background
/// colors.
/// @ingroup dom
Element vscroll_indicator(const Element& child) noexcept {
  class Impl : public NodeDecorator {
    using NodeDecorator::NodeDecorator;

    void ComputeRequirement() noexcept override {
      ftxui::NodeDecorator::ComputeRequirement();
      requirement_ = children_[0]->requirement();
      requirement_.min_x++;
    }

    void SetBox(const Box& box) noexcept override {
      ftxui::NodeDecorator::SetBox(box);
      if (box_.x_min > box_.x_max) {
        box_.x_max--;
      }
      children_[0]->SetBox(box);
    }

    void Render(Screen& screen) noexcept final {
      NodeDecorator::Render(screen);

      const auto& stencil = screen.stencil;

      const int size_inner = box_.y_max - box_.y_min;
      const int size_outter = stencil.y_max - stencil.y_min + 1;
      if (size_outter >= size_inner) {
        return;
      }

      const int size =
          ranges::max(2 * size_outter * size_outter / size_inner, 1);

      const int start_y =
          2 * stencil.y_min +  //
          2 * (stencil.y_min - box_.y_min) * size_outter / size_inner;

      const int x = stencil.x_max;
      for (int y = stencil.y_min; y <= stencil.y_max; ++y) {
        const int y_up = 2 * y + 0;
        const int y_down = 2 * y + 1;
        const bool up = (start_y <= y_up) && (y_up <= start_y + size);
        const bool down = (start_y <= y_down) && (y_down <= start_y + size);

        const char* c = up ? (down ? "┃" : "╹") : (down ? "╻" : " ");  // NOLINT
        screen.PixelAt(x, y) = Pixel();
        screen.PixelAt(x, y).character = c;
      }
    };
  };
  return std::make_unique<Impl>(child);
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
