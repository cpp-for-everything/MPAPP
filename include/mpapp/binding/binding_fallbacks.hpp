// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// MAUI Binding fallbacks — `FallbackValue`, `TargetNullValue`, and
// `StringFormat` lowered into composable, platform-neutral helpers that
// sit alongside `mpapp::binding` WITHOUT touching binding.hpp.
//
// In MAUI a `Binding` resolves a value in three stages:
//   1. If the source path cannot be resolved (no source), the binding
//      emits `FallbackValue`.
//   2. If the source resolves but the value is null, the binding emits
//      `TargetNullValue`.
//   3. Otherwise the resolved value is rendered through `StringFormat`
//      (a "{0}"-style .NET composite format) before reaching the target.
//
// We model the "is the source resolvable?" / "is it null?" distinction
// with `std::optional<T>`:
//   * a disengaged optional  -> no source        -> FallbackValue
//   * an engaged optional    -> source present    -> TargetNullValue is a
//     no-op unless the caller models null separately (see resolve()).
//
// Everything here is header-only template sugar — no macros (ADR-0002),
// no native widget ownership (ADR-0024).

#ifndef MPAPP_BINDING_BINDING_FALLBACKS_HPP
#define MPAPP_BINDING_BINDING_FALLBACKS_HPP

#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_BINDING_FALLBACKS_HAS_STD_FORMAT 1
#endif

namespace mpapp {

// ---- FallbackValue --------------------------------------------------------
//
// MAUI's `FallbackValue`: returned when the source cannot be resolved
// (disengaged optional). When the source IS present, its value is passed
// through unchanged.
template <class T>
[[nodiscard]] T apply_fallbacks(std::optional<T> source, T fallback_value) {
    return source ? std::move(*source) : std::move(fallback_value);
}

// ---- TargetNullValue ------------------------------------------------------
//
// MAUI's `TargetNullValue`: returned when the resolved source value is
// "null". We model null as a disengaged optional too, so a present value
// passes through and an absent value becomes `target_null_value`.
template <class T>
[[nodiscard]] T apply_target_null(std::optional<T> source, T target_null_value) {
    return source ? std::move(*source) : std::move(target_null_value);
}

// ---- StringFormat ---------------------------------------------------------
//
// MAUI's `StringFormat="{0:...}"`. .NET composite formats use a positional
// `{0}` placeholder; std::format uses `{}` / `{0}` too but its format-spec
// syntax differs after the colon. `string_format` accepts a MAUI-style
// pattern, rewrites a leading `{0`/`{0}` placeholder to std::format's
// index-less `{`/`{}` form, and renders the single value through
// std::vformat (so the pattern can be data-driven).
//
// Gated on <format>: Android NDK r26's libc++ does not yet ship it (same
// guard converters.hpp uses). On platforms without <format> the fallback
// helpers above remain available; only string_format is host/desktop-side.
#ifdef MPAPP_BINDING_FALLBACKS_HAS_STD_FORMAT

namespace detail {

// Rewrite MAUI composite-format placeholders into std::format placeholders.
// "{0}"      -> "{}"
// "{0:0.00}" -> "{:0.00}"  (caller is responsible for a std-compatible spec)
// Escaped braces "{{" / "}}" are passed through untouched. Only index 0 is
// supported (a binding renders exactly one value).
[[nodiscard]] inline std::string normalize_maui_format(const std::string& pattern) {
    std::string out;
    out.reserve(pattern.size());
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        const char c = pattern[i];
        if (c == '{') {
            // Escaped "{{" — copy both, skip the second.
            if (i + 1 < pattern.size() && pattern[i + 1] == '{') {
                out += "{{";
                ++i;
                continue;
            }
            // A placeholder "{0" or "{0:..." — drop the "0" index.
            if (i + 1 < pattern.size() && pattern[i + 1] == '0') {
                out += '{';
                ++i;  // skip the '0'
                continue;
            }
            out += c;
            continue;
        }
        if (c == '}' && i + 1 < pattern.size() && pattern[i + 1] == '}') {
            out += "}}";  // escaped "}}"
            ++i;
            continue;
        }
        out += c;
    }
    return out;
}

}  // namespace detail

// Render `value` through a MAUI-style composite format string. The pattern
// must contain a single positional placeholder for index 0 (e.g. "{0}",
// "Total: {0:.2f}"). Returns the formatted string.
template <class T>
[[nodiscard]] std::string string_format(const std::string& fmt, const T& value) {
    const std::string normalized = detail::normalize_maui_format(fmt);
    // std::make_format_args binds to non-const lvalues, so copy.
    T arg = value;
    return std::vformat(normalized, std::make_format_args(arg));
}

#endif  // MPAPP_BINDING_FALLBACKS_HAS_STD_FORMAT

// ---- binding_value resolver ----------------------------------------------
//
// A small composable resolver mirroring MAUI's three-stage value pipeline.
// Populate any combination of `fallback`, `target_null`, and
// `string_format`, then call `resolve(source)`:
//
//   * disengaged source            -> `fallback` (or the type's default if
//                                      no fallback was configured)
//   * engaged source               -> the value, rendered through
//                                      `string_format` when one is set
//
// `target_null` is applied when the source is disengaged AND no `fallback`
// is configured — matching MAUI's precedence where FallbackValue covers an
// unresolved binding and TargetNullValue covers a resolved-but-null value.
template <class T>
struct binding_value {
    std::optional<T>           fallback{};
    std::optional<T>           target_null{};
    std::optional<std::string> string_format{};

    [[nodiscard]] std::string resolve(std::optional<T> source) const {
        if (!source) {
            // Unresolved / null source: prefer FallbackValue, then
            // TargetNullValue, then a default-constructed value.
            const T chosen =
                fallback ? *fallback : (target_null ? *target_null : T{});
            return render(chosen);
        }
        return render(*source);
    }

private:
    [[nodiscard]] std::string render(const T& value) const {
#ifdef MPAPP_BINDING_FALLBACKS_HAS_STD_FORMAT
        if (string_format) {
            return mpapp::string_format(*string_format, value);
        }
        return std::format("{}", value);
#else
        // Without <format> the resolver can only stringify the value when
        // it is already a std::string-convertible type; callers on such
        // platforms should consume the typed helpers above instead.
        if constexpr (std::is_constructible_v<std::string, const T&>) {
            return std::string(value);
        } else {
            return std::string{};
        }
#endif
    }
};

}  // namespace mpapp

#endif  // MPAPP_BINDING_BINDING_FALLBACKS_HPP
