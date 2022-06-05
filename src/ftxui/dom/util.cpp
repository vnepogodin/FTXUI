#include <functional>  // for function
#include <memory>      // for __shared_ptr_access, make_unique
#include <utility>  // for move
#include <vector>   // for vector

#include <ftxui/dom/elements.hpp>     // for Element, Decorator, Elements, operator|, Fit, emptyElement, nothing, operator|=
#include <ftxui/dom/node.hpp>         // for Node, Node::Status
#include <ftxui/dom/requirement.hpp>  // for Requirement
#include <ftxui/screen/box.hpp>       // for Box
#include <ftxui/screen/screen.hpp>    // for Full
#include <ftxui/screen/terminal.hpp>  // for Dimensions

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <range/v3/algorithm/min.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace ftxui {

namespace {
Decorator compose(Decorator a, Decorator b) noexcept {
  return [a = std::move(a), b = std::move(b)](Element element) {
    return b(a(std::move(element)));
  };
}
}  // namespace

/// @brief A decoration doing absolutely nothing.
/// @ingroup dom
Element nothing(Element element) noexcept {
  return element;
}

/// @brief Compose two decorator into one.
/// @ingroup dom
///
/// ### Example
///
/// ```cpp
/// auto decorator = bold | blink;
/// ```
Decorator operator|(Decorator a, Decorator b) noexcept {
  return compose(std::move(a),  //
                 std::move(b));
}

/// @brief From a set of element, apply a decorator to every elements.
/// @return the set of decorated element.
/// @ingroup dom
Elements operator|(Elements elements, const Decorator& decorator) noexcept {  // NOLINT
  Elements output;
  for (auto& it : elements) {
    output.push_back(std::move(it) | decorator);
  }
  return output;
}

/// @brief From an element, apply a decorator.
/// @return the decorated element.
/// @ingroup dom
///
/// ### Example
///
/// Both of these are equivalent:
/// ```cpp
/// bold(text("Hello"));
/// ```
/// ```cpp
/// text("Hello") | bold;
/// ```
Element operator|(Element element, const Decorator& decorator) noexcept {  // NOLINT
  return decorator(std::move(element));
}

/// @brief Apply a decorator to an element.
/// @return the decorated element.
/// @ingroup dom
///
/// ### Example
///
/// Both of these are equivalent:
/// ```cpp
/// auto element = text("Hello");
/// element |= bold;
/// ```
Element& operator|=(Element& e, const Decorator& d) noexcept {
  e = e | d;
  return e;
}

/// The minimal dimension that will fit the given element.
/// @see Fixed
/// @see Full
Dimensions Dimension::Fit(Element& e) noexcept {
  const auto fullsize = Dimension::Full();
  Box box;
  box.x_min = 0;
  box.y_min = 0;
  box.x_max = fullsize.dimx;
  box.y_max = fullsize.dimy;

  Node::Status status;
  e->Check(&status);
  const int max_iteration = 20;
  while (status.need_iteration && status.iteration < max_iteration) {
    e->ComputeRequirement();

    // Don't give the element more space than it needs:
    box.x_max = ranges::min(box.x_max, e->requirement().min_x);
    box.y_max = ranges::min(box.y_max, e->requirement().min_y);

    e->SetBox(box);
    status.need_iteration = false;
    status.iteration++;
    e->Check(&status);

    if (!status.need_iteration) {
      break;
    }
    // Increase the size of the box until it fits, but not more than the with of
    // the terminal emulator:
    box.x_max = ranges::min(e->requirement().min_x, fullsize.dimx);
    box.y_max = ranges::min(e->requirement().min_y, fullsize.dimy);
  }

  return {
      box.x_max,
      box.y_max,
  };
}

/// An element of size 0x0 drawing nothing.
/// @ingroup dom
Element emptyElement() noexcept {
  class Impl : public Node {
    void ComputeRequirement() noexcept override {
      requirement_.min_x = 0;
      requirement_.min_x = 0;
    }
  };
  return std::make_unique<Impl>();
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
