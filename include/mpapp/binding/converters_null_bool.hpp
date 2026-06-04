// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// Null/empty-checking value converters — CTK batch 1.
// Mirrors CommunityToolkit.Maui semantics:
//   is_null              — true when std::optional is empty or pointer is null
//   is_not_null          — true when std::optional has value or pointer is non-null
//   is_not_null_or_empty — true when std::string is non-empty, or
//                          std::optional<std::string> has a non-empty value
//   is_string_not_null_or_whitespace
//                        — true when the string contains at least one
//                          non-whitespace character (std::string or
//                          std::optional<std::string>)
// `invert_bool` already lives in converters.hpp; it is not repeated here.
// No macros (ADR-0002). Platform-neutral, header-only.

#ifndef MPAPP_BINDING_CONVERTERS_NULL_BOOL_HPP
#define MPAPP_BINDING_CONVERTERS_NULL_BOOL_HPP

#include <algorithm>
#include <functional>
#include <optional>
#include <string>

#include "binding.hpp"   // for mpapp::value_converter

namespace mpapp {

// ---------------------------------------------------------------------------
// is_null — std::optional<T>
// ---------------------------------------------------------------------------

template <class T>
struct is_null_converter : value_converter<std::optional<T>, bool> {
    [[nodiscard]] bool convert(const std::optional<T>& source) const override {
        return !source.has_value();
    }
    [[nodiscard]] std::optional<T> convert_back(const bool& /*target*/) const override {
        return std::nullopt;
    }
};

template <class T>
[[nodiscard]] inline std::function<bool(const std::optional<T>&)> is_null() {
    return [](const std::optional<T>& v) { return !v.has_value(); };
}

// is_null — raw pointer overload
template <class T>
struct is_null_ptr_converter : value_converter<T*, bool> {
    [[nodiscard]] bool convert(T* const& source) const override {
        return source == nullptr;
    }
    [[nodiscard]] T* convert_back(const bool& /*target*/) const override {
        return nullptr;
    }
};

template <class T>
[[nodiscard]] inline std::function<bool(T* const&)> is_null_ptr() {
    return [](T* const& p) { return p == nullptr; };
}

// ---------------------------------------------------------------------------
// is_not_null — std::optional<T>
// ---------------------------------------------------------------------------

template <class T>
struct is_not_null_converter : value_converter<std::optional<T>, bool> {
    [[nodiscard]] bool convert(const std::optional<T>& source) const override {
        return source.has_value();
    }
    [[nodiscard]] std::optional<T> convert_back(const bool& /*target*/) const override {
        return std::nullopt;
    }
};

template <class T>
[[nodiscard]] inline std::function<bool(const std::optional<T>&)> is_not_null() {
    return [](const std::optional<T>& v) { return v.has_value(); };
}

// is_not_null — raw pointer overload
template <class T>
struct is_not_null_ptr_converter : value_converter<T*, bool> {
    [[nodiscard]] bool convert(T* const& source) const override {
        return source != nullptr;
    }
    [[nodiscard]] T* convert_back(const bool& /*target*/) const override {
        return nullptr;
    }
};

template <class T>
[[nodiscard]] inline std::function<bool(T* const&)> is_not_null_ptr() {
    return [](T* const& p) { return p != nullptr; };
}

// ---------------------------------------------------------------------------
// is_not_null_or_empty
//   • std::string           — non-empty string
//   • std::optional<string> — has value AND that value is non-empty
// ---------------------------------------------------------------------------

struct is_not_null_or_empty_converter : value_converter<std::string, bool> {
    [[nodiscard]] bool convert(const std::string& source) const override {
        return !source.empty();
    }
    [[nodiscard]] std::string convert_back(const bool& /*target*/) const override {
        return {};
    }
};

[[nodiscard]] inline std::function<bool(const std::string&)> is_not_null_or_empty() {
    return [](const std::string& s) { return !s.empty(); };
}

struct is_not_null_or_empty_opt_converter
    : value_converter<std::optional<std::string>, bool> {
    [[nodiscard]] bool convert(const std::optional<std::string>& source) const override {
        return source.has_value() && !source->empty();
    }
    [[nodiscard]] std::optional<std::string>
    convert_back(const bool& /*target*/) const override {
        return std::nullopt;
    }
};

[[nodiscard]] inline
std::function<bool(const std::optional<std::string>&)> is_not_null_or_empty_opt() {
    return [](const std::optional<std::string>& s) {
        return s.has_value() && !s->empty();
    };
}

// ---------------------------------------------------------------------------
// is_string_not_null_or_whitespace
//   • std::string           — at least one non-whitespace character
//   • std::optional<string> — has value AND contains non-whitespace
// ---------------------------------------------------------------------------

namespace detail {
    inline bool has_non_whitespace(const std::string& s) {
        return std::any_of(s.begin(), s.end(),
            [](unsigned char c) { return !std::isspace(c); });
    }
} // namespace detail

struct is_string_not_null_or_whitespace_converter : value_converter<std::string, bool> {
    [[nodiscard]] bool convert(const std::string& source) const override {
        return detail::has_non_whitespace(source);
    }
    [[nodiscard]] std::string convert_back(const bool& /*target*/) const override {
        return {};
    }
};

[[nodiscard]] inline
std::function<bool(const std::string&)> is_string_not_null_or_whitespace() {
    return [](const std::string& s) { return detail::has_non_whitespace(s); };
}

struct is_string_not_null_or_whitespace_opt_converter
    : value_converter<std::optional<std::string>, bool> {
    [[nodiscard]] bool convert(const std::optional<std::string>& source) const override {
        return source.has_value() && detail::has_non_whitespace(*source);
    }
    [[nodiscard]] std::optional<std::string>
    convert_back(const bool& /*target*/) const override {
        return std::nullopt;
    }
};

[[nodiscard]] inline
std::function<bool(const std::optional<std::string>&)>
is_string_not_null_or_whitespace_opt() {
    return [](const std::optional<std::string>& s) {
        return s.has_value() && detail::has_non_whitespace(*s);
    };
}

} // namespace mpapp

#endif // MPAPP_BINDING_CONVERTERS_NULL_BOOL_HPP
