#ifndef FTXUI_DOM_DEPRECATED_HPP
#define FTXUI_DOM_DEPRECATED_HPP

#include "ftxui/dom/elements.hpp"

namespace ftxui {
Element text(const std::wstring& text) noexcept;
Element vtext(const std::wstring& text) noexcept;
Elements paragraph(std::wstring text) noexcept;
}  // namespace ftxui

#endif  // FTXUI_DOM_DEPRECATED_HPP

// Copyright 2021 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
