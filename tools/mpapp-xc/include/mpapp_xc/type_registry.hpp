// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Tag -> C++ type and attribute -> setter registry for the mpapp-xc XAML
// emitter. Maps MAUI-flavored XAML element names (e.g. "VerticalStackLayout",
// "Label") onto the concrete `mpapp::` C++ types they lower to, plus the
// child-wiring kind (layout / single-content / page / leaf) and the header
// each type needs. Attributes are mapped onto value-lowering rules so the
// emitter can produce the right C++ statement for each `Name="value"` pair.
//
// Unknown tags / attributes are reported as `std::nullopt` so the emitter can
// degrade to a `// TODO` comment without aborting the whole file.

#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace mpapp::xc {

// How an element wires its children into the generated visual tree.
enum class child_kind {
    // Multi-child container; children attach via `parent.add(child)`.
    layout,
    // Single-child decorator; child attaches via `parent.content = child`.
    single_content,
    // Page; its single content view attaches via `parent.content = child`.
    page,
    // No children expected (Label, Button, Entry, ...).
    leaf,
};

// Resolved type information for a supported XAML element.
struct element_type_info {
    // The fully-qualified C++ type the element lowers to
    // (e.g. "mpapp::internal::basic_stack_layout").
    std::string_view cpp_type{};
    // The mpapp public header that declares `cpp_type`.
    std::string_view header{};
    // How children of this element are wired.
    child_kind kind{child_kind::leaf};
};

// Iterate the distinct mpapp headers needed by the supported element set.
// `callback(header)` is invoked once per header in declaration order. Used by
// the emitter to write a deduplicated `#include` block.
void for_each_element_header(void (*callback)(std::string_view header, void* user),
                             void* user);

// How an attribute's value is lowered into a C++ initializer expression.
enum class value_kind {
    // Quoted string literal: Text="Hi" -> "Hi"
    string,
    // Floating-point literal: FontSize="18" -> 18.0
    number,
    // Boolean literal: IsToggled="true" -> true
    boolean,
    // Uniform thickness: Padding="24" -> mpapp::thickness{24}
    thickness,
    // Orientation enum: Orientation="Vertical" -> mpapp::orientation::vertical
    orientation_enum,
};

// Resolved setter information for a supported attribute on a given element.
struct attribute_setter_info {
    // The C++ member to assign (e.g. "text", "spacing", "stack_orientation").
    std::string_view member{};
    value_kind kind{value_kind::string};
};

// Look up the C++ type / child-wiring for a XAML element tag. Returns
// std::nullopt for tags the registry does not know about.
std::optional<element_type_info> lookup_element(std::string_view tag) noexcept;

// Look up the setter rule for an attribute on a given (already-resolved)
// element tag. Returns std::nullopt for attributes the registry does not
// support (including XAML directives like `x:Class`). The element tag is
// taken into account so the same attribute name can map differently per
// element where required.
std::optional<attribute_setter_info> lookup_attribute(std::string_view tag,
                                                      std::string_view attribute) noexcept;

// Lower an attribute value into a C++ initializer expression per `kind`.
// `raw` is the verbatim XAML attribute value. The returned string is a
// complete right-hand-side expression (e.g. `"Hello"`, `18.0`, `true`,
// `mpapp::thickness{24}`, `mpapp::orientation::vertical`).
std::string lower_value(value_kind kind, std::string_view raw);

} // namespace mpapp::xc
