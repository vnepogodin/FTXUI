#ifndef FTXUI_DOM_TAKE_ANY_ARGS_HPP
#define FTXUI_DOM_TAKE_ANY_ARGS_HPP

// IWYU pragma: private, include "ftxui/dom/elements.hpp"
#include <type_traits>
#include <algorithm>

namespace ftxui {

template <class T>
void Merge(Elements& /*container*/, T&& /*element*/) {}

template <>
inline void Merge(Elements& container, Element&& element) {
  container.push_back(std::move(element));
}

template <>
inline void Merge(Elements& container, const Elements& elements) {
  std::copy(elements.begin(), elements.end(),
          std::back_inserter(container));
}

// Turn a set of arguments into a vector.
template <class... Args>
Elements unpack(Args... args) {
  std::vector<Element> vec;
  static_assert((std::is_constructible_v<Element, Args> && ...));
  (Merge(vec, std::forward<Args>(args)), ...);
  return vec;
}

// Make |container| able to take any number of argments.
#define TAKE_ANY_ARGS(container)                               \
  template <class... Args>                                     \
  Element container(Args... children) {                        \
    return container(unpack(std::forward<Args>(children)...)); \
  }

TAKE_ANY_ARGS(vbox)
TAKE_ANY_ARGS(hbox)
TAKE_ANY_ARGS(dbox)
TAKE_ANY_ARGS(hflow)
}  // namespace ftxui

#endif  // FTXUI_DOM_TAKE_ANY_ARGS_HPP

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
