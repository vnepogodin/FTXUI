#include <functional>  // for function
#include <utility>

#include "ftxui/component/component.hpp"  // for Renderer, ComponentDecorator, ElementDecorator, operator|, operator|=
#include "ftxui/component/component_base.hpp"  // for Component

namespace ftxui {

// NOLINTNEXTLINE
Component operator|(const Component& component,
                    const ComponentDecorator& decorator) noexcept {
  return decorator(component);
}

// NOLINTNEXTLINE
Component operator|(const Component& component, const ElementDecorator& decorator) noexcept {
  return component | Renderer(decorator);
}

// NOLINTNEXTLINE
Component& operator|=(Component& component,
                      const ComponentDecorator& decorator) noexcept {
  component = component | decorator;
  return component;
}

// NOLINTNEXTLINE
Component& operator|=(Component& component,
                      const ElementDecorator& decorator) noexcept {
  component = component | decorator;
  return component;
}

}  // namespace ftxui

// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
