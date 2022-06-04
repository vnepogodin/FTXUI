#ifndef FTXUI_COMPONENT_HPP
#define FTXUI_COMPONENT_HPP

#include <functional>  // for function
#include <memory>      // for make_shared, shared_ptr
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
constexpr std::shared_ptr<T> Make(Args&&... args) noexcept {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

// Pipe operator to decorate components.
using ComponentDecorator = std::function<Component(Component)>;
using ElementDecorator = std::function<Element(Element)>;
Component operator|(Component component, const ComponentDecorator& decorator) noexcept;
Component operator|(Component component, ElementDecorator decorator) noexcept;
Component& operator|=(Component& component, const ComponentDecorator& decorator) noexcept;
Component& operator|=(Component& component, ElementDecorator decorator) noexcept;

namespace Container {
Component Vertical(Components children) noexcept;
Component Vertical(Components children, int* selector) noexcept;
Component Horizontal(Components children) noexcept;
Component Horizontal(Components children, int* selector) noexcept;
Component Tab(Components children, int* selector) noexcept;

}  // namespace Container

Component Button(const ConstStringRef& label,
                 std::function<void()> on_click,
                 Ref<ButtonOption> = ButtonOption::Simple()) noexcept;

Component Checkbox(const ConstStringRef& label,
                   bool* checked,
                   Ref<CheckboxOption> option = CheckboxOption::Simple()) noexcept;

Component Input(const StringRef& content,
                const ConstStringRef& placeholder,
                Ref<InputOption> option = {}) noexcept;

Component Menu(ConstStringListRef entries,
               int* selected_,
               Ref<MenuOption> = MenuOption::Vertical()) noexcept;
Component MenuEntry(ConstStringRef label, Ref<MenuEntryOption> = {}) noexcept;

Component Dropdown(ConstStringListRef entries, int* selected) noexcept;

Component Radiobox(ConstStringListRef entries,
                   int* selected_,
                   Ref<RadioboxOption> option = {}) noexcept;
Component Toggle(ConstStringListRef entries, int* selected) noexcept;

template <class T>  // T = {int, float, long}
Component Slider(ConstStringRef label, T* value, T min, T max, T increment) noexcept;

Component ResizableSplitLeft(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitRight(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitTop(const Component& main, const Component& back, int* main_size) noexcept;
Component ResizableSplitBottom(const Component& main, const Component& back, int* main_size) noexcept;

Component Renderer(Component child, std::function<Element()>) noexcept;
Component Renderer(std::function<Element()>) noexcept;
Component Renderer(const std::function<Element(bool /* focused */)>&) noexcept;
ComponentDecorator Renderer(ElementDecorator) noexcept;

Component CatchEvent(const Component& child, const std::function<bool(const Event&)>&) noexcept;
ComponentDecorator CatchEvent(const std::function<bool(const Event&)>& on_event) noexcept;

Component Maybe(Component, const bool* show) noexcept;
Component Maybe(Component, const std::function<bool()>&) noexcept;
ComponentDecorator Maybe(const bool* show) noexcept;
ComponentDecorator Maybe(std::function<bool()>) noexcept;

Component Collapsible(const ConstStringRef& label,
                      const Component& child,
                      Ref<bool> show = false) noexcept;
}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_HPP */

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
