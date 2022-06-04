#ifndef FTXUI_DOM_ELEMENTS_HPP
#define FTXUI_DOM_ELEMENTS_HPP

#include <functional>
#include <memory>

#include "ftxui/dom/canvas.hpp"
#include "ftxui/dom/flexbox_config.hpp"
#include "ftxui/dom/node.hpp"
#include "ftxui/screen/box.hpp"
#include "ftxui/screen/color.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/terminal.hpp"
#include "ftxui/util/ref.hpp"

namespace ftxui {
class Node;
using Element = std::shared_ptr<Node>;
using Elements = std::vector<Element>;
using Decorator = std::function<Element(Element)>;
using GraphFunction = std::function<std::vector<int>(int, int)>;

enum BorderStyle { LIGHT, HEAVY, DOUBLE, ROUNDED, EMPTY };
enum class GaugeDirection { Left, Up, Right, Down };

// Pipe elements into decorator togethers.
// For instance the next lines are equivalents:
// -> text("ftxui") | bold | underlined
// -> underlined(bold(text("FTXUI")))
Element operator|(Element, const Decorator&) noexcept;
Element& operator|=(Element&, const Decorator&) noexcept;
Elements operator|(Elements, const Decorator&) noexcept;
Decorator operator|(Decorator, Decorator) noexcept;

// --- Widget ---
Element text(const char* text) noexcept;
Element text(std::string_view text) noexcept;
Element text(const std::string& text) noexcept;
Element vtext(const std::string& text) noexcept;
Element separator() noexcept;
Element separatorLight() noexcept;
Element separatorHeavy() noexcept;
Element separatorDouble() noexcept;
Element separatorEmpty() noexcept;
Element separatorStyled(BorderStyle) noexcept;
Element separator(const Pixel&) noexcept;
Element separatorCharacter(const std::string&) noexcept;
Element separatorHSelector(float left,
                           float right,
                           Color background,
                           Color foreground) noexcept;
Element separatorVSelector(float up,
                           float down,
                           Color background,
                           Color foreground) noexcept;
Element gauge(float ratio) noexcept;
Element gaugeLeft(float ratio) noexcept;
Element gaugeRight(float ratio) noexcept;
Element gaugeUp(float ratio) noexcept;
Element gaugeDown(float ratio) noexcept;
Element gaugeDirection(float ratio, GaugeDirection) noexcept;
Element border(Element) noexcept;
Element borderLight(Element) noexcept;
Element borderHeavy(Element) noexcept;
Element borderDouble(Element) noexcept;
Element borderRounded(Element) noexcept;
Element borderEmpty(Element) noexcept;
Decorator borderStyled(BorderStyle) noexcept;
Decorator borderWith(const Pixel&) noexcept;
Element window(Element title, Element content) noexcept;
Element spinner(int charset_index, size_t image_index) noexcept;
Element paragraph(const std::string& text) noexcept;
Element paragraphAlignLeft(const std::string& text) noexcept;
Element paragraphAlignRight(const std::string& text) noexcept;
Element paragraphAlignCenter(const std::string& text) noexcept;
Element paragraphAlignJustify(const std::string& text) noexcept;
Element graph(const GraphFunction&) noexcept;
Element emptyElement() noexcept;
Element canvas(ConstRef<Canvas>) noexcept;
Element canvas(int width, int height, std::function<void(Canvas&)>) noexcept;
Element canvas(std::function<void(Canvas&)>) noexcept;

// -- Decorator ---
Element bold(Element) noexcept;
Element dim(Element) noexcept;
Element inverted(Element) noexcept;
Element underlined(Element) noexcept;
Element blink(Element) noexcept;
Decorator color(Color) noexcept;
Decorator bgcolor(Color) noexcept;
Element color(Color, Element) noexcept;
Element bgcolor(Color, Element) noexcept;
Decorator focusPosition(int x, int y) noexcept;
Decorator focusPositionRelative(float x, float y) noexcept;
Element automerge(Element) noexcept;

// --- Layout is
// Horizontal, Vertical or stacked set of elements.
Element hbox(Elements) noexcept;
Element vbox(Elements) noexcept;
Element dbox(Elements) noexcept;
Element flexbox(Elements, FlexboxConfig config = FlexboxConfig()) noexcept;
Element gridbox(std::vector<Elements> lines) noexcept;

Element hflow(Elements) noexcept;  // Helper: default flexbox with row direction.
Element vflow(Elements) noexcept;  // Helper: default flexbox with column direction.

// -- Flexibility ---
// Define how to share the remaining space when not all of it is used inside a
// container.
Element flex(Element) noexcept;         // Expand/Minimize if possible/needed.
Element flex_grow(Element) noexcept;    // Expand element if possible.
Element flex_shrink(Element) noexcept;  // Minimize element if needed.

Element xflex(Element) noexcept;         // Expand/Minimize if possible/needed on X axis.
Element xflex_grow(Element) noexcept;    // Expand element if possible on X axis.
Element xflex_shrink(Element) noexcept;  // Minimize element if needed on X axis.

Element yflex(Element) noexcept;         // Expand/Minimize if possible/needed on Y axis.
Element yflex_grow(Element) noexcept;    // Expand element if possible on Y axis.
Element yflex_shrink(Element) noexcept;  // Minimize element if needed on Y axis.

Element notflex(Element) noexcept;  // Reset the flex attribute.
Element filler() noexcept;          // A blank expandable element.

// -- Size override;
enum Direction { WIDTH, HEIGHT };
enum Constraint { LESS_THAN, EQUAL, GREATER_THAN };
Decorator size(Direction, Constraint, int value) noexcept;

// --
Decorator reflect(Box& box) noexcept;

// --- Frame ---
// A frame is a scrollable area. The internal area is potentially larger than
// the external one. The internal area is scrolled in order to make visible the
// focused element.
Element frame(Element) noexcept;
Element xframe(Element) noexcept;
Element yframe(Element) noexcept;
Element focus(Element) noexcept;
Element select(Element) noexcept;

Element vscroll_indicator(Element) noexcept;

// --- Util --------------------------------------------------------------------
Element hcenter(Element) noexcept;
Element vcenter(Element) noexcept;
Element center(Element) noexcept;
Element align_right(Element) noexcept;
Element nothing(Element element) noexcept;

// Before drawing the |element| clear the pixel below. This is useful in
// combinaison with dbox.
Element clear_under(Element element) noexcept;

namespace Dimension {
Dimensions Fit(Element&) noexcept;
}  // namespace Dimension

}  // namespace ftxui

// Make container able to take any number of children as input.
#include <ftxui/dom/take_any_args.hpp>

// Include old definitions using wstring.
#include <ftxui/dom/deprecated.hpp>
#endif  // FTXUI_DOM_ELEMENTS_HPP

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
