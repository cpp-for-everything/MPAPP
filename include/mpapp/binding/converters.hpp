// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// A small library of ready-made value converters for data binding —
// the workhorses every MVVM app needs (bool inversion, bool->visibility,
// runtime string formatting). Each is both a `value_converter<S,T>`
// object (MAUI's IValueConverter shape, for resource registration) and a
// free-function helper that returns the plain `std::function` a
// `binding<S,T>` takes in its converter slot. No macros; platform-neutral.

#ifndef MPAPP_BINDING_CONVERTERS_HPP
#define MPAPP_BINDING_CONVERTERS_HPP

#include <functional>
#include <string>
#include <utility>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_CONVERTERS_HAS_STD_FORMAT 1
#endif

#include "../view.hpp"   // for mpapp::visibility
#include "binding.hpp"   // for mpapp::value_converter

namespace mpapp {

// ---- bool <-> bool (negation) ---------------------------------------------

struct invert_bool_converter : value_converter<bool, bool> {
    [[nodiscard]] bool convert(const bool& source) const override { return !source; }
    [[nodiscard]] bool convert_back(const bool& target) const override { return !target; }
};

[[nodiscard]] inline std::function<bool(const bool&)> invert_bool() {
    return [](const bool& b) { return !b; };
}

// ---- bool <-> visibility ---------------------------------------------------

struct bool_to_visibility_converter : value_converter<bool, visibility> {
    // `collapse_when_false`: false -> collapsed (default) vs hidden.
    explicit bool_to_visibility_converter(bool collapse_when_false = true)
        : collapse_{ collapse_when_false } {}

    [[nodiscard]] visibility convert(const bool& source) const override {
        if (source) return visibility::visible;
        return collapse_ ? visibility::collapsed : visibility::hidden;
    }
    [[nodiscard]] bool convert_back(const visibility& target) const override {
        return target == visibility::visible;
    }

private:
    bool collapse_;
};

[[nodiscard]] inline std::function<visibility(const bool&)>
bool_to_visibility(bool collapse_when_false = true) {
    return [collapse_when_false](const bool& b) {
        if (b) return visibility::visible;
        return collapse_when_false ? visibility::collapsed : visibility::hidden;
    };
}

// ---- value -> string (runtime StringFormat) -------------------------------
//
// Gated on <format>: Android NDK r26's libc++ does not yet ship it (same
// guard view.hpp uses). The bool/visibility converters above stay
// available on every platform; the string-format converters are
// host/desktop-side (their main consumer is the XAML StringFormat
// lowering, which runs in the mpapp-xc host tool).
#ifdef MPAPP_CONVERTERS_HAS_STD_FORMAT

// MAUI's StringFormat="{0:...}". `pattern` is a std::format string with a
// single `{}` placeholder, applied at runtime via std::vformat (so the
// pattern can be data-driven). Example: format_with<double>("{:.2f}").
template <class T>
[[nodiscard]] std::function<std::string(const T&)> format_with(std::string pattern) {
    return [pattern = std::move(pattern)](const T& v) {
        // std::make_format_args binds to non-const lvalues, so copy.
        T arg = v;
        return std::vformat(pattern, std::make_format_args(arg));
    };
}

// Generic value -> string via the default "{}" formatter.
template <class T>
[[nodiscard]] std::function<std::string(const T&)> to_string_converter() {
    return [](const T& v) { return std::format("{}", v); };
}

#endif // MPAPP_CONVERTERS_HAS_STD_FORMAT

} // namespace mpapp

#endif // MPAPP_BINDING_CONVERTERS_HPP
