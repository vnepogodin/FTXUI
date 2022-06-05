#include <memory>     // for __shared_ptr_access
#include <string>       // for string
#include <string_view>  // for string_view
#include <utility>      // for move

#include <ftxui/component/component.hpp>  // for Maybe, Checkbox, Make, Radiobox, Vertical, Dropdown
#include <ftxui/component/component_base.hpp>  // for Component, ComponentBase
#include <ftxui/component/component_options.hpp>  // for CheckboxOption, EntryState
#include <ftxui/dom/elements.hpp>  // for operator|, Element, border, filler, operator|=, separator, size, text, vbox, frame, vscroll_indicator, hbox, HEIGHT, LESS_THAN, bold, inverted
#include <ftxui/util/ref.hpp>      // for ConstStringListRef

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif

#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/min.hpp>

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace ftxui {

Component Dropdown(ConstStringListRef entries, int* selected) noexcept {
  class Impl : public ComponentBase {
   public:
    Impl(ConstStringListRef entries, int* selected)
        : entries_(entries), selected_(selected) {
      CheckboxOption option;
      option.transform = [](auto&& s) {
        const auto prefix = text(s.state ? "↓ " : "→ ");
        auto t = text(s.label);
        if (s.active) {
          t |= bold;
        }
        if (s.focused) {
          t |= inverted;
        }
        return hbox({prefix, t});
      };
      checkbox_ = Checkbox(&title_, &show_, option),
      radiobox_ = Radiobox(entries_, selected_);

      Add(Container::Vertical({
          checkbox_,
          Maybe(radiobox_, &show_),
      }));
    }

    Element Render() noexcept override {
      *selected_ = ranges::min(static_cast<int>(entries_.size()) - 1,
                               ranges::max(0, *selected_));
      title_ = entries_[*selected_];
      if (show_) {
        static constexpr int max_height = 12;
        return vbox({
                   checkbox_->Render(),
                   separator(),
                   radiobox_->Render() | vscroll_indicator | frame |
                       size(HEIGHT, LESS_THAN, max_height),
               }) |
               border;
      }

      return vbox({
          checkbox_->Render() | border,
          filler(),
      });
    }

   private:
    ConstStringListRef entries_;
    bool show_{false};
    int* selected_{};
    std::string title_{};
    Component checkbox_{};
    Component radiobox_{};
  };

  return Make<Impl>(entries, selected);
}

}  // namespace ftxui

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
