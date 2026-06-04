// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// CTK-style value converters (batch 3). Complements converters.hpp
// (bool/visibility/string-format) and converters_value_map.hpp (bool->T,
// enum->bool, int->bool, index<->array) with predicate and arithmetic
// converters commonly needed in MAUI/CTK view-model wiring:
//
//   is_equal<T>      — true when source == target value
//   is_not_equal<T>  — true when source != target value
//   is_in_range<T>   — true when min <= source <= max
//   clamp<T>         — clamps source into [min, max]
//   bool_negation    — wraps operator! on a bool (lightweight, no state)
//   double_to_int    — round-to-nearest, returns int
//   int_to_double    — widens int to double
//   string_to_upper  — locale-independent ASCII upper-case
//   string_to_lower  — locale-independent ASCII lower-case
//
// Each type is both a `value_converter<S,T>` object (for resource
// dictionaries) and a self-contained callable, so it can be handed
// directly to `binding<S,T>`'s converter slot. No macros; platform-
// neutral; header-only.

#ifndef MPAPP_BINDING_CONVERTERS_CTK2_HPP
#define MPAPP_BINDING_CONVERTERS_CTK2_HPP

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <utility>

#include "binding.hpp"   // for mpapp::value_converter

namespace mpapp {

// ---- T -> bool (equality predicate) ----------------------------------------
//
// Returns true when source == target_.
// convert_back: returns target_ (the stored sentinel value).

template <class T>
struct is_equal : value_converter<T, bool> {
    T target_{};

    is_equal() = default;
    explicit is_equal(T target) : target_{ std::move(target) } {}

    [[nodiscard]] bool convert(const T& source) const override {
        return source == target_;
    }
    [[nodiscard]] T convert_back(const bool& /*result*/) const override {
        return target_;
    }

    [[nodiscard]] bool operator()(const T& source) const {
        return source == target_;
    }
};

// ---- T -> bool (inequality predicate) --------------------------------------
//
// Returns true when source != target_.
// convert_back: returns target_ (the stored sentinel value).

template <class T>
struct is_not_equal : value_converter<T, bool> {
    T target_{};

    is_not_equal() = default;
    explicit is_not_equal(T target) : target_{ std::move(target) } {}

    [[nodiscard]] bool convert(const T& source) const override {
        return source != target_;
    }
    [[nodiscard]] T convert_back(const bool& /*result*/) const override {
        return target_;
    }

    [[nodiscard]] bool operator()(const T& source) const {
        return source != target_;
    }
};

// ---- T -> bool (range predicate) -------------------------------------------
//
// Returns true when min_ <= source <= max_.
// convert_back: clamps target back to [min_, max_] and returns it.

template <class T>
struct is_in_range : value_converter<T, bool> {
    T min_{};
    T max_{};

    is_in_range() = default;
    is_in_range(T min_val, T max_val)
        : min_{ std::move(min_val) }
        , max_{ std::move(max_val) } {}

    [[nodiscard]] bool convert(const T& source) const override {
        return source >= min_ && source <= max_;
    }
    [[nodiscard]] T convert_back(const bool& /*result*/) const override {
        return min_;
    }

    [[nodiscard]] bool operator()(const T& source) const {
        return source >= min_ && source <= max_;
    }
};

// ---- T -> T (clamp) --------------------------------------------------------
//
// Clamps source into the closed interval [min_, max_].
// convert_back: identity (clamp is idempotent on a value already in range,
// and there is no unique inverse for out-of-range values).

template <class T>
struct clamp : value_converter<T, T> {
    T min_{};
    T max_{};

    clamp() = default;
    clamp(T min_val, T max_val)
        : min_{ std::move(min_val) }
        , max_{ std::move(max_val) } {}

    [[nodiscard]] T convert(const T& source) const override {
        return std::clamp(source, min_, max_);
    }
    [[nodiscard]] T convert_back(const T& target) const override {
        return std::clamp(target, min_, max_);
    }

    [[nodiscard]] T operator()(const T& source) const {
        return std::clamp(source, min_, max_);
    }
};

// ---- bool -> bool (negation) -----------------------------------------------
//
// Lightweight stateless wrapper for operator!. Analogous to
// invert_bool_converter in converters.hpp but expressed as a plain
// callable struct without inheriting from value_converter, matching
// the CTK NegateConverter shape. Use invert_bool_converter when you need
// a polymorphic value_converter object; use bool_negation when you only
// need a callable (e.g. as a binding converter lambda).

struct bool_negation : value_converter<bool, bool> {
    [[nodiscard]] bool convert(const bool& source) const override {
        return !source;
    }
    [[nodiscard]] bool convert_back(const bool& target) const override {
        return !target;
    }

    [[nodiscard]] bool operator()(bool source) const {
        return !source;
    }
};

// ---- double -> int (round-to-nearest) --------------------------------------
//
// Rounds source to the nearest integer using std::lround.
// convert_back: widens int back to double (exact).

struct double_to_int : value_converter<double, int> {
    [[nodiscard]] int convert(const double& source) const override {
        return static_cast<int>(std::lround(source));
    }
    [[nodiscard]] double convert_back(const int& target) const override {
        return static_cast<double>(target);
    }

    [[nodiscard]] int operator()(double source) const {
        return static_cast<int>(std::lround(source));
    }
};

// ---- int -> double (widening) ----------------------------------------------
//
// Widens an int to double (exact for all 32-bit integers).
// convert_back: rounds back using std::lround.

struct int_to_double : value_converter<int, double> {
    [[nodiscard]] double convert(const int& source) const override {
        return static_cast<double>(source);
    }
    [[nodiscard]] int convert_back(const double& target) const override {
        return static_cast<int>(std::lround(target));
    }

    [[nodiscard]] double operator()(int source) const {
        return static_cast<double>(source);
    }
};

// ---- string -> string (ASCII upper-case) -----------------------------------
//
// Converts each character in source to its upper-case equivalent using
// the C locale (locale-independent for ASCII). Non-ASCII bytes are passed
// through unchanged.
// convert_back: applies lower-case (round-trip for ASCII alpha only).

struct string_to_upper : value_converter<std::string, std::string> {
    [[nodiscard]] std::string convert(const std::string& source) const override {
        std::string out;
        out.reserve(source.size());
        for (unsigned char ch : source) {
            out += static_cast<char>(std::toupper(ch));
        }
        return out;
    }
    [[nodiscard]] std::string convert_back(const std::string& target) const override {
        std::string out;
        out.reserve(target.size());
        for (unsigned char ch : target) {
            out += static_cast<char>(std::tolower(ch));
        }
        return out;
    }

    [[nodiscard]] std::string operator()(const std::string& source) const {
        return convert(source);
    }
};

// ---- string -> string (ASCII lower-case) -----------------------------------
//
// Converts each character in source to its lower-case equivalent using
// the C locale (locale-independent for ASCII). Non-ASCII bytes are passed
// through unchanged.
// convert_back: applies upper-case (round-trip for ASCII alpha only).

struct string_to_lower : value_converter<std::string, std::string> {
    [[nodiscard]] std::string convert(const std::string& source) const override {
        std::string out;
        out.reserve(source.size());
        for (unsigned char ch : source) {
            out += static_cast<char>(std::tolower(ch));
        }
        return out;
    }
    [[nodiscard]] std::string convert_back(const std::string& target) const override {
        std::string out;
        out.reserve(target.size());
        for (unsigned char ch : target) {
            out += static_cast<char>(std::toupper(ch));
        }
        return out;
    }

    [[nodiscard]] std::string operator()(const std::string& source) const {
        return convert(source);
    }
};

} // namespace mpapp

#endif // MPAPP_BINDING_CONVERTERS_CTK2_HPP
