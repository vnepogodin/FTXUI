#if defined(__clang__)
#include <sstream>  // for basic_istream, stringstream
#include <string>   // for string, allocator, getline
#else
#include <ranges>
#endif
#include <string>   // for string, allocator
#include <utility>  // for move
#include <algorithm>
#include <string_view>

#include <ftxui/dom/elements.hpp>  // for flexbox, Element, text, Elements, operator|, xflex, paragraph, paragraphAlignCenter, paragraphAlignJustify, paragraphAlignLeft, paragraphAlignRight
#include <ftxui/dom/flexbox_config.hpp>  // for FlexboxConfig, FlexboxConfig::JustifyContent, FlexboxConfig::JustifyContent::Center, FlexboxConfig::JustifyContent::FlexEnd, FlexboxConfig::JustifyContent::SpaceBetween

namespace ftxui {

namespace {
#if defined(__clang__)
Elements Split(const std::string& the_text) noexcept {
  Elements output;
  std::stringstream ss{the_text};
  std::string word;
  while (std::getline(ss, word, ' '))
    output.emplace_back(text(word));

  return output;
}
#else
namespace ranges = std::ranges;
Elements Split(const std::string_view&& the_text) noexcept {
  static constexpr std::string_view delim{" "};
  static constexpr auto functor = [](auto&& rng) {
    return std::string_view(&*rng.begin(), static_cast<size_t>(ranges::distance(rng)));
  };
  static constexpr auto second = [](auto&& rng) { return rng != ""; };

  const auto& view_res = the_text
                         | ranges::views::split(delim)
                         | ranges::views::transform(functor);

  Elements output{};
  ranges::for_each(view_res | ranges::views::filter(second), [&](auto&& rng) { output.emplace_back(text(rng)); });
  return output;
}
#endif
}  // namespace

/// @brief Return an element drawing the paragraph on multiple lines.
/// @ingroup dom
/// @see flexbox.
Element paragraph(const std::string& the_text) noexcept {
  return paragraphAlignLeft(the_text);
}

/// @brief Return an element drawing the paragraph on multiple lines, aligned on
/// the left.
/// @ingroup dom
/// @see flexbox.
Element paragraphAlignLeft(const std::string& the_text) noexcept {
  static const auto config = FlexboxConfig().SetGap(1, 0);
  return flexbox(Split(the_text), config);
}

/// @brief Return an element drawing the paragraph on multiple lines, aligned on
/// the right.
/// @ingroup dom
/// @see flexbox.
Element paragraphAlignRight(const std::string& the_text) noexcept {
  static const auto config =
      FlexboxConfig().SetGap(1, 0).Set(FlexboxConfig::JustifyContent::FlexEnd);
  return flexbox(Split(the_text), config);
}

/// @brief Return an element drawing the paragraph on multiple lines, aligned on
/// the center.
/// @ingroup dom
/// @see flexbox.
Element paragraphAlignCenter(const std::string& the_text) noexcept {
  static const auto config =
      FlexboxConfig().SetGap(1, 0).Set(FlexboxConfig::JustifyContent::Center);
  return flexbox(Split(the_text), config);
}

/// @brief Return an element drawing the paragraph on multiple lines, aligned
/// using a justified alignment.
/// the center.
/// @ingroup dom
/// @see flexbox.
Element paragraphAlignJustify(const std::string& the_text) noexcept {
  static const auto config = FlexboxConfig().SetGap(1, 0).Set(
      FlexboxConfig::JustifyContent::SpaceBetween);
  Elements words = Split(the_text);
  words.push_back(text(std::string_view{}) | xflex);
  return flexbox(std::move(words), config);
}

}  // namespace ftxui

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Copyright 2022 Vladislav Nepogodin. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
