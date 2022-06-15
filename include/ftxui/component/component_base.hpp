#ifndef FTXUI_COMPONENT_BASE_HPP
#define FTXUI_COMPONENT_BASE_HPP

#include <memory>  // for unique_ptr
#include <vector>  // for vector

#include "ftxui/component/captured_mouse.hpp"  // for CaptureMouse
#include "ftxui/dom/elements.hpp"              // for Element

namespace ftxui {

class Focus;
struct Event;

namespace animation {
class Params;
}  // namespace animation

class ComponentBase;
using Component = std::shared_ptr<ComponentBase>;
using Components = std::vector<Component>;

/// @brief It implement rendering itself as ftxui::Element. It implement
/// keyboard navigation by responding to ftxui::Event.
/// @ingroup component
class ComponentBase {
 public:
  // virtual Destructor.
  virtual ~ComponentBase() noexcept;

  // Component hierarchy:
  [[nodiscard]] ComponentBase* Parent() const noexcept;
  Component& ChildAt(size_t i) noexcept;
  [[nodiscard]] std::size_t ChildCount() const noexcept;
  void Add(const Component& children) noexcept;
  void Detach() noexcept;
  void DetachAllChildren() noexcept;

  // Renders the component.
  virtual Element Render() noexcept;

  // Handles an event.
  // By default, reduce on children with a lazy OR.
  //
  // Returns whether the event was handled or not.
  virtual bool OnEvent(const Event&) noexcept;

  // Handle an animation step.
  virtual void OnAnimation(animation::Params& params) noexcept;

  // Focus management ----------------------------------------------------------
  //
  // If this component contains children, this indicates which one is active,
  // nullptr if none is active.
  //
  // We say an element has the focus if the chain of ActiveChild() from the
  // root component contains this object.
  virtual Component ActiveChild() noexcept;

  // Return true when the component contains focusable elements.
  // The non focusable Component will be skipped when navigating using the
  // keyboard.
  [[nodiscard]] virtual bool Focusable() const noexcept;

  // Whether this is the active child of its parent.
  [[nodiscard]] bool Active() const noexcept;
  // Whether all the ancestors are active.
  [[nodiscard]] bool Focused() const noexcept;

  // Make the |child| to be the "active" one.
  virtual void SetActiveChild(const ComponentBase* child) noexcept;
  void SetActiveChild(const Component& child) noexcept;

  // Configure all the ancestors to give focus to this component.
  void TakeFocus() noexcept;

 protected:
  CapturedMouse CaptureMouse(const Event& event) noexcept;

  Components children_;

 private:
  ComponentBase* parent_ = nullptr;
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_BASE_HPP */

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
