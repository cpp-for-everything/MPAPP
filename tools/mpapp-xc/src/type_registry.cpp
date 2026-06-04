// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.

#include "mpapp_xc/type_registry.hpp"

#include <array>
#include <string>
#include <string_view>

namespace mpapp::xc {

namespace {

struct element_row {
    std::string_view tag;
    std::string_view cpp_type;
    std::string_view header;
    child_kind kind;
};

// Supported XAML elements -> mpapp C++ types.
//
// The generated code targets the cross-platform `internal::basic_*` surfaces
// (the real mpapp public API per ADR-0008 — the same bases the `mpapp::label`
// etc. wrappers publicly derive from). The basic surfaces carry the identical
// property / `add()` / `content` API but DO NOT embed the per-OS native
// handler, so the generated header compiles standalone on any host (the
// wrappers would otherwise require the per-platform SDK, e.g. winrt on
// Windows). This is exactly the "host-side validation harness" surface called
// out in <mpapp/mpapp.hpp>.
//
// MAUI's VerticalStackLayout / HorizontalStackLayout / StackLayout all lower
// to basic_stack_layout; the orientation difference is applied by the emitter.
constexpr std::array element_table{
    element_row{"ContentPage", "mpapp::internal::basic_content_page",
                "mpapp/internal/basic_content_page.hpp", child_kind::page},
    element_row{"VerticalStackLayout", "mpapp::internal::basic_stack_layout",
                "mpapp/internal/basic_stack_layout.hpp", child_kind::layout},
    element_row{"HorizontalStackLayout", "mpapp::internal::basic_stack_layout",
                "mpapp/internal/basic_stack_layout.hpp", child_kind::layout},
    element_row{"StackLayout", "mpapp::internal::basic_stack_layout",
                "mpapp/internal/basic_stack_layout.hpp", child_kind::layout},
    element_row{"Grid", "mpapp::internal::basic_grid_layout",
                "mpapp/internal/basic_grid_layout.hpp", child_kind::layout},
    element_row{"Label", "mpapp::internal::basic_label",
                "mpapp/internal/basic_label.hpp", child_kind::leaf},
    element_row{"Button", "mpapp::internal::basic_button",
                "mpapp/internal/basic_button.hpp", child_kind::leaf},
    element_row{"Entry", "mpapp::internal::basic_entry",
                "mpapp/internal/basic_entry.hpp", child_kind::leaf},
    element_row{"Editor", "mpapp::internal::basic_editor",
                "mpapp/internal/basic_editor.hpp", child_kind::leaf},
    element_row{"Switch", "mpapp::internal::basic_switch_",
                "mpapp/internal/basic_switch_.hpp", child_kind::leaf},
    element_row{"Slider", "mpapp::internal::basic_slider",
                "mpapp/internal/basic_slider.hpp", child_kind::leaf},
    element_row{"CheckBox", "mpapp::internal::basic_check_box",
                "mpapp/internal/basic_check_box.hpp", child_kind::leaf},
    element_row{"Image", "mpapp::internal::basic_image",
                "mpapp/internal/basic_image.hpp", child_kind::leaf},
    element_row{"BoxView", "mpapp::internal::basic_box_view",
                "mpapp/internal/basic_box_view.hpp", child_kind::leaf},
    element_row{"Border", "mpapp::internal::basic_border",
                "mpapp/internal/basic_border.hpp", child_kind::single_content},
    element_row{"ScrollView", "mpapp::internal::basic_scroll_view",
                "mpapp/internal/basic_scroll_view.hpp", child_kind::single_content},
};

struct attribute_row {
    // Empty `tag` matches any element (shared view-level attributes).
    std::string_view tag;
    std::string_view attribute;
    std::string_view member;
    value_kind kind;
};

// Attribute -> setter rules. Element-specific rows are listed before the
// wildcard rows so a per-element mapping wins over the shared view-level
// one. Only properties that exist on the corresponding mpapp surface are
// listed; everything else degrades to a `// TODO` in the emitter.
constexpr std::array attribute_table{
    // --- Label ---------------------------------------------------------
    attribute_row{"Label", "Text", "text", value_kind::string},
    attribute_row{"Label", "FontSize", "font_size", value_kind::number},
    // --- Button --------------------------------------------------------
    attribute_row{"Button", "Text", "text", value_kind::string},
    // --- Entry / Editor ------------------------------------------------
    attribute_row{"Entry", "Text", "text", value_kind::string},
    attribute_row{"Entry", "Placeholder", "placeholder", value_kind::string},
    attribute_row{"Entry", "IsPassword", "is_password", value_kind::boolean},
    attribute_row{"Entry", "IsReadOnly", "is_read_only", value_kind::boolean},
    attribute_row{"Editor", "Text", "text", value_kind::string},
    // --- Switch --------------------------------------------------------
    attribute_row{"Switch", "IsToggled", "is_on", value_kind::boolean},
    // --- Slider --------------------------------------------------------
    attribute_row{"Slider", "Value", "value", value_kind::number},
    attribute_row{"Slider", "Minimum", "minimum", value_kind::number},
    attribute_row{"Slider", "Maximum", "maximum", value_kind::number},
    // --- CheckBox ------------------------------------------------------
    attribute_row{"CheckBox", "IsChecked", "is_checked", value_kind::boolean},
    // --- Image ---------------------------------------------------------
    attribute_row{"Image", "Source", "source", value_kind::string},
    // --- Stack layout --------------------------------------------------
    attribute_row{"VerticalStackLayout", "Spacing", "spacing", value_kind::number},
    attribute_row{"HorizontalStackLayout", "Spacing", "spacing", value_kind::number},
    attribute_row{"StackLayout", "Spacing", "spacing", value_kind::number},
    attribute_row{"StackLayout", "Orientation", "stack_orientation",
                  value_kind::orientation_enum},
    // --- Border / ScrollView padding -----------------------------------
    attribute_row{"Border", "StrokeThickness", "stroke_thickness", value_kind::number},
    // --- Shared view-level attributes (wildcard tag) -------------------
    attribute_row{"", "Padding", "padding", value_kind::thickness},
    attribute_row{"", "WidthRequest", "width", value_kind::number},
    attribute_row{"", "HeightRequest", "height", value_kind::number},
    attribute_row{"", "Opacity", "opacity", value_kind::number},
    attribute_row{"", "IsEnabled", "is_enabled", value_kind::boolean},
    attribute_row{"", "AutomationId", "automation_id", value_kind::string},
};

// Escape a XAML string value into a C++ string literal body (without the
// surrounding quotes). Keeps the generated source well-formed regardless of
// embedded quotes / backslashes / newlines.
std::string escape_string(std::string_view raw) {
    std::string out{};
    out.reserve(raw.size() + 2);
    for (char c : raw) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '"': out += "\\\""; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

// Trim ASCII whitespace from both ends.
std::string_view trim(std::string_view s) noexcept {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

bool iequals(std::string_view a, std::string_view b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb - 'A' + 'a');
        if (ca != cb) return false;
    }
    return true;
}

} // namespace

std::optional<element_type_info> lookup_element(std::string_view tag) noexcept {
    for (const auto& row : element_table) {
        if (row.tag == tag) {
            return element_type_info{row.cpp_type, row.header, row.kind};
        }
    }
    return std::nullopt;
}

void for_each_element_header(void (*callback)(std::string_view, void*), void* user) {
    for (const auto& row : element_table) {
        callback(row.header, user);
    }
}

std::optional<attribute_setter_info> lookup_attribute(std::string_view tag,
                                                      std::string_view attribute) noexcept {
    // XAML directives and namespace declarations are never lowered.
    if (attribute == "x:Class" || attribute == "xmlns" ||
        attribute.starts_with("xmlns:")) {
        return std::nullopt;
    }
    // Element-specific rows first, then wildcard (empty tag) rows.
    for (const auto& row : attribute_table) {
        if (row.tag == tag && row.attribute == attribute) {
            return attribute_setter_info{row.member, row.kind};
        }
    }
    for (const auto& row : attribute_table) {
        if (row.tag.empty() && row.attribute == attribute) {
            return attribute_setter_info{row.member, row.kind};
        }
    }
    return std::nullopt;
}

std::string lower_value(value_kind kind, std::string_view raw) {
    const std::string_view value = trim(raw);
    switch (kind) {
        case value_kind::string:
            return std::string{"\""} + escape_string(value) + "\"";
        case value_kind::number: {
            std::string out{value};
            // Force a floating-point literal so it binds to Observable<double>.
            if (out.find('.') == std::string::npos &&
                out.find('e') == std::string::npos &&
                out.find('E') == std::string::npos) {
                out += ".0";
            }
            return out;
        }
        case value_kind::boolean:
            return iequals(value, "true") ? "true" : "false";
        case value_kind::thickness:
            return std::string{"mpapp::thickness{"} + std::string{value} + "}";
        case value_kind::orientation_enum:
            return iequals(value, "Horizontal")
                       ? "mpapp::orientation::horizontal"
                       : "mpapp::orientation::vertical";
    }
    return std::string{"\""} + escape_string(value) + "\"";
}

} // namespace mpapp::xc
