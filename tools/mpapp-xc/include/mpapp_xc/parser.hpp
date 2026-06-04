// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// XAML parser for the mpapp-xc compiler.
//
// The skeleton parses just enough of a MAUI-flavored XAML file to capture:
//   * the root element's tag name,
//   * xmlns / xmlns:x namespace declarations,
//   * the x:Class directive,
//   * and a recording (as raw strings) of any other root attributes / children.
//
// Markup-extension expansion, binding resolution, and the rest of the lowering
// pipeline land in P3 (see vault/10_Architecture/Markup.md). This file is
// scaffolding only.

#pragma once

#include "diagnostics.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mpapp::xc {

struct xaml_attribute {
    std::string name{};
    std::string value{};
};

struct xaml_element {
    std::string tag{};
    std::vector<xaml_attribute> attributes{};
    // 1-based source line where this element opens. 0 means unknown -- the
    // emitter falls back to a sensible default. Drives the `#line` directives
    // so generated-code diagnostics surface at the original XAML location.
    std::ptrdiff_t line{0};
    // Child elements, lowered recursively by the emitter.
    std::vector<xaml_element> children{};
};

struct xaml_document {
    std::filesystem::path source_path{};

    // The XAML default namespace -- typically the MAUI namespace URI.
    std::optional<std::string> default_xmlns{};

    // The xmlns:x prefix -- typically the XAML language URI
    // (http://schemas.microsoft.com/winfx/2009/xaml).
    std::optional<std::string> x_namespace{};

    // The x:Class directive value (e.g. "MyApp.MainPage"), if present.
    std::optional<std::string> x_class{};

    // True if the document had a root element at all.
    bool has_root{false};

    // 1-based line where the root element opens in the source. 0 means
    // unknown -- the emitter falls back to a sensible default.
    std::ptrdiff_t root_line{0};

    xaml_element root{};
};

// Parse a XAML source buffer into an `xaml_document`.
//
// Errors are recorded in `diagnostics`; the function does NOT throw. On a
// fatal parse failure the returned document has `has_root == false`.
xaml_document parse(std::string_view source,
                    std::filesystem::path source_path,
                    diagnostic_collector& diagnostics);

// Helper: extract the unqualified C++ identifier from an x:Class value.
// `"MyApp.MainPage"` becomes `"MainPage"`. Returns empty if `x_class` is empty
// or contains no usable trailing identifier character.
std::string class_basename(std::string_view x_class) noexcept;

} // namespace mpapp::xc
