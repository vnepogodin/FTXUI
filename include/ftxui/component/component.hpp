#ifndef FTXUI_COMPONENT_HPP
#define FTXUI_COMPONENT_HPP

#include <functional>  // for function
#include <memory>      // for make_unique, shared_ptr
#include <string>      // for wstring
#include <utility>     // for forward
#include <vector>      // for vector

#include <ftxui/component/component_base.hpp>     // for Component, Components
#include <ftxui/component/component_options.hpp>  // for ButtonOption, CheckboxOption, InputOption (ptr only), MenuEntryOption (ptr only), MenuOption, RadioboxOption (ptr only)
#include <ftxui/dom/elements.hpp>                 // for Element
#include <ftxui/util/ref.hpp>  // for Ref, ConstStringRef, ConstStringListRef, StringRef

namespace ftxui {
struct ButtonOption;
struct CheckboxOption;
struct Event;
struct InputOption;
struct MenuOption;
struct RadioboxOption;
struct MenuEntryOption;

template <class T, class... Args>
constexpr std::unique_ptr<T> Make(Args&&... args) noexcept {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

// Pipe operator to decorate components.
using ComponentDecorator = std::function<Component(const Component&)>;
using ElementDecorator = std::function<Element(const Element&)>;
Component operator|(const Component& component, const ComponentDecorator& decorator) noexcept;
Component operator|(const Component& component, const ElementDecorator& decorator) noexcept;
Component& operator|=(Component& component, const ComponentDecorator& decorator) noexcept;
Component& operator|=(Component& component, const ElementDecorator& decorator) noexcept;

namespace Container {
Component Vertical(const Components& children) noexcept;
Component Vertical(const Components& children, int* selector) noexcept;
Component Horizontal(const Components& children) noexcept;
Component Horizontal(const Components& children, int* selector) noexcept;
Component Tab(const Components& children, int* selector) noexcept;

}  // namespace Container

Component Button(const ConstStringRef& label,
                 const std::function<void()>& on_click,
                 const Ref<ButtonOption>& = ButtonOption::Simple()) noexcept;

Component Checkbox(const ConstStringRef& label,
                   bool* checked,
                   const Ref<CheckboxOption>& option = CheckboxOption::Simple()) noexcept;

Component Input(StringRef content,
                ConstStringRef placeholder,
                const Ref<InputOption>& option = {}) noexcept;

Component Menu(ConstStringListRef entries,
               int* selected_,
               const Ref<MenuOption>& = MenuOption::Vertical()) noexcept;
Component MenuEntry(const ConstStringRef& label, const Ref<MenuEntryOption>& = {}) noexcept;

Component Dropdown(ConstStringListRef entries, int* selected) noexcept;

Component Radiobox(ConstStringListRef entries,
                   int* selected_,
                   const Ref<RadioboxOption>& option = {}) noexcept;
Component Toggle(ConstStringListRef entries, int* selected) noexcept;

template <class T>  // T = {int, float, long}
Component Slider(ConstStringRef label, T* value, T min, T max, T increment) noexcept;

Component ResizableSplitLeft(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitRight(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitTop(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitBottom(const Component& main, const Component& back, int* main_size) noexcept;

Component Renderer(const Component& child, const std::function<Element()>&) noexcept;
Component Renderer(const std::function<Element()>&) noexcept;
Component Renderer(const std::function<Element(bool /* focused */)>&) noexcept;
ComponentDecorator Renderer(const ElementDecorator&) noexcept;

Component CatchEvent(const Component& child, const std::function<bool(const Event&)>&) noexcept;
ComponentDecorator CatchEvent(const std::function<bool(const Event&)>& on_event) noexcept;

Component Maybe(const Component&, const bool* show) noexcept;
Component Maybe(const Component&, const std::function<bool()>&) noexcept;
ComponentDecorator Maybe(const bool* show) noexcept;
ComponentDecorator Maybe(const std::function<bool()>&) noexcept;

Component Modal(const Component& main, const Component& modal, const bool* show_modal) noexcept;
ComponentDecorator Modal(const Component& modal, const bool* show_modal) noexcept;

Component Collapsible(const ConstStringRef& label,
                      const Component& child,
                      Ref<bool> show = false) noexcept;
}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_HPP */

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
