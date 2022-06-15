#include <functional>  // for function
#include <memory>      // for shared_ptr
#include <utility>     // for move

#include "ftxui/component/animation.hpp"  // for Animator, Params (ptr only)
#include "ftxui/component/captured_mouse.hpp"  // for CapturedMouse
#include "ftxui/component/component.hpp"       // for Make, Button
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for ButtonOption, AnimatedColorOption, AnimatedColorsOption, EntryState
#include "ftxui/component/event.hpp"  // for Event, Event::Return
#include "ftxui/component/mouse.hpp"  // for Mouse, Mouse::Left, Mouse::Pressed
#include "ftxui/component/screen_interactive.hpp"  // for Component
#include "ftxui/dom/elements.hpp"  // for operator|, Decorator, Element, operator|=, bgcolor, color, reflect, text, bold, border, inverted, nothing
#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/color.hpp"  // for Color
#include "ftxui/util/ref.hpp"      // for Ref, ConstStringRef

namespace ftxui {

namespace {

Element DefaultTransform(const EntryState& params) noexcept {  // NOLINT
  auto element = text(params.label) | border;
  if (params.active) {
    element |= bold;
  }
  if (params.focused) {
    element |= inverted;
  }
  return element;
}

}  // namespace

/// @brief Draw a button. Execute a function when clicked.
/// @param label The label of the button.
/// @param on_click The action to execute when clicked.
/// @param option Additional optional parameters.
/// @ingroup component
/// @see ButtonBase
///
/// ### Example
///
/// ```cpp
/// auto screen = ScreenInteractive::FitComponent();
/// std::string label = "Click to quit";
/// Component button = Button(&label, screen.ExitLoopClosure());
/// screen.Loop(button)
/// ```
///
/// ### Output
///
/// ```bash
/// ┌─────────────┐
/// │Click to quit│
/// └─────────────┘
/// ```
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
Component Button(ConstStringRef label,
                 const std::function<void()>& on_click,
                 const Ref<ButtonOption>& option) noexcept {
  class Impl : public ComponentBase {
   public:
    Impl(ConstStringRef label,
         const std::function<void()>& on_click,
         const Ref<ButtonOption>& option)
        : label_(label),
          option_(option),
          on_click_(on_click) {}

    // Component implementation:
    Element Render() noexcept override {
      const bool active = Active();
      const bool focused = Focused();
      const bool focused_or_hover = focused || mouse_hover_;

      const float target = focused_or_hover ? 1.F : 0.F;  // NOLINT
      if (target != animator_background_.to()) {
        SetAnimationTarget(target);
      }

      const auto focus_management = focused ? focus : active ? select : nothing;
      const EntryState state = {
          false,
          active,
          focused_or_hover,
          *label_,
      };

      const auto element =
          (option_->transform ? option_->transform : DefaultTransform)  //
          (state);
      return element | AnimatedColorStyle() | focus_management | reflect(box_);
    }

    Decorator AnimatedColorStyle() noexcept {
      Decorator style = nothing;
      if (option_->animated_colors.background.enabled) {
        style = style | bgcolor(Color::Interpolate(
                            animation_foreground_,  //
                            option_->animated_colors.background.inactive,
                            option_->animated_colors.background.active));
      }
      if (option_->animated_colors.foreground.enabled) {
        style = style | color(Color::Interpolate(
                            animation_foreground_,  //
                            option_->animated_colors.foreground.inactive,
                            option_->animated_colors.foreground.active));
      }
      return style;
    }

    void SetAnimationTarget(float target) {
      if (option_->animated_colors.foreground.enabled) {
        animator_foreground_ =
            animation::Animator(&animation_foreground_, target,
                                option_->animated_colors.foreground.duration,
                                option_->animated_colors.foreground.function);
      }
      if (option_->animated_colors.background.enabled) {
        animator_background_ =
            animation::Animator(&animation_background_, target,
                                option_->animated_colors.background.duration,
                                option_->animated_colors.background.function);
      }
    }

    void OnAnimation(animation::Params& p) noexcept override {
      animator_background_.OnAnimation(p);
      animator_foreground_.OnAnimation(p);
    }

    void OnClick() noexcept {
      on_click_();
      animation_background_ = 0.5F;  // NOLINT
      animation_foreground_ = 0.5F;  // NOLINT
      SetAnimationTarget(1.F);       // NOLINT
    }

    bool OnEvent(const Event& event) noexcept override {
      if (event.is_mouse()) {
        return OnMouseEvent(event);
      }

      if (event == Event::Return) {
        OnClick();
        return true;
      }
      return false;
    }

    bool OnMouseEvent(const Event& event) noexcept {
      mouse_hover_ =
          box_.Contain(event.mouse().x, event.mouse().y) && CaptureMouse(event);

      if (!mouse_hover_) {
        return false;
      }

      if (event.mouse().button == Mouse::Button::Left &&
          event.mouse().motion == Mouse::Motion::Pressed) {
        TakeFocus();
        OnClick();
        return true;
      }

      return false;
    }

    [[nodiscard]] bool Focusable() const noexcept final { return true; }

   private:
    bool mouse_hover_ = false;
    float animation_background_{};
    float animation_foreground_{};
    ConstStringRef label_;
    Box box_{};
    Ref<ButtonOption> option_;
    std::function<void()> on_click_;
    animation::Animator animator_background_ =
        animation::Animator(&animation_background_);
    animation::Animator animator_foreground_ =
        animation::Animator(&animation_foreground_);
  };

  return std::make_unique<Impl>(label, on_click, option);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
