// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// XAML -> C++ emitter for mpapp-xc.
//
// The skeleton emitter produces a `consteval` function whose body is a stub
// placeholder. The function name is derived from the x:Class basename. Each
// emitted statement is prefixed with a `#line` directive pointing back at the
// original XAML so downstream C++ compilation errors surface at the XAML
// source location (see vault/10_Architecture/Markup.md).

#pragma once

#include "parser.hpp"

#include <string>

namespace mpapp::xc {

struct emit_options {
    // Filename to use in #line directives. When empty, the document's
    // `source_path.filename()` is used.
    std::string source_filename{};
};

std::string emit(const xaml_document& doc, const emit_options& options = {});

} // namespace mpapp::xc
