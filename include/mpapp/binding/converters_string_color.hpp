// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// CTK converters batch 3 — string/color helpers:
//   color_to_hex            color -> "#RRGGBB" or "#RRGGBBAA"
//   color_to_rgba_string    color -> "rgba(r,g,b,a)"  (human-readable CSS-ish)
//   text_case               string -> string (none/upper/lower/title-case)
//   list_to_string          std::vector<std::string> -> joined string
//   string_to_list          string -> std::vector<std::string> (split on sep)
//
// All converters follow the same dual-flavour pattern as converters.hpp:
// a `value_converter<S,T>` object for resource-dictionary registration AND
// a free-function helper returning the plain `std::function` that a
// `binding<S,T>` takes in its converter slot.  No macros.  Platform-neutral.

#ifndef MPAPP_BINDING_CONVERTERS_STRING_COLOR_HPP
#define MPAPP_BINDING_CONVERTERS_STRING_COLOR_HPP

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "../color.hpp"
#include "binding.hpp"   // for mpapp::value_converter

namespace mpapp {

// ---------------------------------------------------------------------------
// Internal helpers (not part of the public API surface)
// ---------------------------------------------------------------------------
namespace detail {

// Clamp a double channel [0,1] and round to an 8-bit byte.
inline int to_byte(double ch) noexcept {
    if (ch <= 0.0) return 0;
    if (ch >= 1.0) return 255;
    return static_cast<int>(ch * 255.0 + 0.5);
}

// Format a single byte as exactly two uppercase hex digits.
inline std::string hex2(int byte_val) {
    char buf[3];
    static const char kHex[] = "0123456789ABCDEF";
    buf[0] = kHex[(byte_val >> 4) & 0xF];
    buf[1] = kHex[byte_val & 0xF];
    buf[2] = '\0';
    return std::string(buf, 2);
}

} // namespace detail

// ---------------------------------------------------------------------------
// color_to_hex  —  color -> "#RRGGBB" or "#RRGGBBAA"
//
// When `include_alpha` is true (or when alpha < 1.0 and the two-arg
// free-function overload deduces opacity), the eight-character form is
// produced; otherwise the six-character "#RRGGBB" form is used.
// ---------------------------------------------------------------------------

struct color_to_hex_converter : value_converter<color, std::string> {
    explicit color_to_hex_converter(bool include_alpha = false)
        : include_alpha_{ include_alpha } {}

    [[nodiscard]] std::string convert(const color& c) const override {
        using detail::to_byte;
        using detail::hex2;
        std::string s;
        s.reserve(9);
        s += '#';
        s += hex2(to_byte(c.r));
        s += hex2(to_byte(c.g));
        s += hex2(to_byte(c.b));
        if (include_alpha_) {
            s += hex2(to_byte(c.a));
        }
        return s;
    }

    // Reverse: "#RRGGBB" or "#RRGGBBAA" -> color.
    // Non-conforming strings produce a zero-initialized color (a = 1 default).
    [[nodiscard]] color convert_back(const std::string& hex) const override {
        if (hex.empty() || hex[0] != '#') return color{};
        auto parse2 = [&](std::size_t pos) -> int {
            if (pos + 1 >= hex.size()) return 0;
            auto nibble = [](char ch) -> int {
                if (ch >= '0' && ch <= '9') return ch - '0';
                if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
                if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
                return 0;
            };
            return (nibble(hex[pos]) << 4) | nibble(hex[pos + 1]);
        };
        color c;
        c.r = parse2(1) / 255.0;
        c.g = parse2(3) / 255.0;
        c.b = parse2(5) / 255.0;
        c.a = (hex.size() >= 9) ? (parse2(7) / 255.0) : 1.0;
        return c;
    }

private:
    bool include_alpha_;
};

// Free-function helpers for use directly as converter slots in binding<>.

[[nodiscard]] inline std::function<std::string(const color&)>
color_to_hex(bool include_alpha = false) {
    return [include_alpha](const color& c) {
        using detail::to_byte;
        using detail::hex2;
        std::string s;
        s.reserve(9);
        s += '#';
        s += hex2(to_byte(c.r));
        s += hex2(to_byte(c.g));
        s += hex2(to_byte(c.b));
        if (include_alpha) s += hex2(to_byte(c.a));
        return s;
    };
}

// ---------------------------------------------------------------------------
// color_to_rgba_string  —  color -> "rgba(r,g,b,a)"  (0–255, alpha 0–255)
//
// Matches the format already established by the color std::formatter in
// color.hpp *except* that each channel is rendered as an integer byte
// (0–255) rather than a raw double, which is the CSS/Android convention.
// ---------------------------------------------------------------------------

struct color_to_rgba_string_converter : value_converter<color, std::string> {
    [[nodiscard]] std::string convert(const color& c) const override {
        using detail::to_byte;
        std::ostringstream oss;
        oss << "rgba("
            << to_byte(c.r) << ','
            << to_byte(c.g) << ','
            << to_byte(c.b) << ','
            << to_byte(c.a) << ')';
        return oss.str();
    }

    // Reverse is not meaningful in the general case; return black-transparent.
    [[nodiscard]] color convert_back(const std::string&) const override {
        return color{};
    }
};

[[nodiscard]] inline std::function<std::string(const color&)>
color_to_rgba_string() {
    return [](const color& c) {
        using detail::to_byte;
        std::ostringstream oss;
        oss << "rgba("
            << to_byte(c.r) << ','
            << to_byte(c.g) << ','
            << to_byte(c.b) << ','
            << to_byte(c.a) << ')';
        return oss.str();
    };
}

// ---------------------------------------------------------------------------
// text_case — transform a string to none / upper / lower / title case
// ---------------------------------------------------------------------------

enum class text_case : std::uint8_t {
    none  = 0,
    upper = 1,
    lower = 2,
    title = 3,
};

struct text_case_converter : value_converter<std::string, std::string> {
    explicit text_case_converter(text_case mode = text_case::none)
        : mode_{ mode } {}

    [[nodiscard]] std::string convert(const std::string& s) const override {
        return apply(s, mode_);
    }

    // Lossy (lower/upper is not invertible); returns unchanged.
    [[nodiscard]] std::string convert_back(const std::string& s) const override {
        return s;
    }

    static std::string apply(const std::string& s, text_case mode) {
        switch (mode) {
            case text_case::upper: {
                std::string out(s.size(), '\0');
                std::transform(s.begin(), s.end(), out.begin(),
                               [](unsigned char ch) {
                                   return static_cast<char>(std::toupper(ch));
                               });
                return out;
            }
            case text_case::lower: {
                std::string out(s.size(), '\0');
                std::transform(s.begin(), s.end(), out.begin(),
                               [](unsigned char ch) {
                                   return static_cast<char>(std::tolower(ch));
                               });
                return out;
            }
            case text_case::title: {
                std::string out(s);
                bool next_cap = true;
                for (auto& ch : out) {
                    unsigned char uch = static_cast<unsigned char>(ch);
                    if (std::isspace(uch)) {
                        next_cap = true;
                    } else if (next_cap) {
                        ch = static_cast<char>(std::toupper(uch));
                        next_cap = false;
                    } else {
                        ch = static_cast<char>(std::tolower(uch));
                    }
                }
                return out;
            }
            default:
                return s;
        }
    }

private:
    text_case mode_;
};

[[nodiscard]] inline std::function<std::string(const std::string&)>
make_text_case(text_case mode) {
    return [mode](const std::string& s) {
        return text_case_converter::apply(s, mode);
    };
}

// ---------------------------------------------------------------------------
// list_to_string — join a std::vector<std::string> with a separator
// ---------------------------------------------------------------------------

struct list_to_string_converter : value_converter<std::vector<std::string>, std::string> {
    explicit list_to_string_converter(std::string separator = ",")
        : separator_{ std::move(separator) } {}

    [[nodiscard]] std::string convert(const std::vector<std::string>& list) const override {
        return join(list, separator_);
    }

    [[nodiscard]] std::vector<std::string> convert_back(const std::string& s) const override {
        return split(s, separator_);
    }

    [[nodiscard]] const std::string& separator() const noexcept { return separator_; }

    static std::string join(const std::vector<std::string>& list,
                            const std::string& sep) {
        std::string out;
        for (std::size_t i = 0; i < list.size(); ++i) {
            if (i > 0) out += sep;
            out += list[i];
        }
        return out;
    }

    static std::vector<std::string> split(const std::string& s,
                                          const std::string& sep) {
        std::vector<std::string> result;
        if (sep.empty()) {
            // empty separator: each character becomes one element
            for (char ch : s) result.push_back(std::string(1, ch));
            return result;
        }
        std::size_t start = 0;
        while (true) {
            std::size_t pos = s.find(sep, start);
            if (pos == std::string::npos) {
                result.push_back(s.substr(start));
                break;
            }
            result.push_back(s.substr(start, pos - start));
            start = pos + sep.size();
        }
        return result;
    }

private:
    std::string separator_;
};

[[nodiscard]] inline std::function<std::string(const std::vector<std::string>&)>
list_to_string(std::string separator = ",") {
    return [sep = std::move(separator)](const std::vector<std::string>& list) {
        return list_to_string_converter::join(list, sep);
    };
}

// ---------------------------------------------------------------------------
// string_to_list — split a string on a separator
// ---------------------------------------------------------------------------

struct string_to_list_converter : value_converter<std::string, std::vector<std::string>> {
    explicit string_to_list_converter(std::string separator = ",")
        : separator_{ std::move(separator) } {}

    [[nodiscard]] std::vector<std::string> convert(const std::string& s) const override {
        return list_to_string_converter::split(s, separator_);
    }

    [[nodiscard]] std::string convert_back(const std::vector<std::string>& list) const override {
        return list_to_string_converter::join(list, separator_);
    }

    [[nodiscard]] const std::string& separator() const noexcept { return separator_; }

private:
    std::string separator_;
};

[[nodiscard]] inline std::function<std::vector<std::string>(const std::string&)>
string_to_list(std::string separator = ",") {
    return [sep = std::move(separator)](const std::string& s) {
        return list_to_string_converter::split(s, sep);
    };
}

} // namespace mpapp

#endif // MPAPP_BINDING_CONVERTERS_STRING_COLOR_HPP
