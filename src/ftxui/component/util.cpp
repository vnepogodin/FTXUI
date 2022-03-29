#include <functional>  // for function
#include <utility>

#include "ftxui/component/component.hpp"  // for Renderer, ComponentDecorator, ElementDecorator, operator|, operator|=
#include "ftxui/component/component_base.hpp"  // for Component

namespace ftxui {

Component operator|(Component component,
                    const ComponentDecorator& decorator) noexcept {
  return decorator(std::move(component));
}

Component operator|(Component component, ElementDecorator decorator) noexcept {
  return std::move(component) | Renderer(std::move(decorator));
}

Component& operator|=(Component& component,
                      const ComponentDecorator& decorator) noexcept {
  component = component | decorator;
  return component;
}

Component& operator|=(Component& component,
                      ElementDecorator decorator) noexcept {
  component = component | std::move(decorator);
  return component;
}

}  // namespace ftxui

// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
