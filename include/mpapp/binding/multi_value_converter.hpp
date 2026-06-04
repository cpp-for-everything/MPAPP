// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// `mpapp::multi_value_converter<Inputs, Output>` — MAUI's IMultiValueConverter
// shape for multi-source bindings: takes a `std::vector<Input>` (homogeneous)
// or a parameter-pack form and produces one `Output`. Companion to
// `multi_binding<T, Ss...>`.
//
// Also provides:
//   - `all_true_converter`   — bool-vector AND reduction
//   - `any_true_converter`   — bool-vector OR reduction
//   - `concat_converter`     — string-vector join with a separator
//   - `chain_converter(c1, c2)` — compose two single-value converters:
//     output of c1 (S -> M) feeds c2 (M -> T), yielding S -> T
//
// All types are header-only, platform-neutral, and contain no macros
// (ADR-0002 / ADR-0009).

#ifndef MPAPP_BINDING_MULTI_VALUE_CONVERTER_HPP
#define MPAPP_BINDING_MULTI_VALUE_CONVERTER_HPP

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace mpapp {

// ---------------------------------------------------------------------------
// Abstract base: IMultiValueConverter (homogeneous-input variant)
// ---------------------------------------------------------------------------
//
// Concrete converters inherit and implement `convert`. The `Input` and
// `Output` type parameters mirror MAUI's generic IMultiValueConverter<T, R>.

template <class Input, class Output>
struct multi_value_converter {
    virtual ~multi_value_converter() = default;

    [[nodiscard]] virtual Output
    convert(const std::vector<Input>& values) const = 0;
};

// ---------------------------------------------------------------------------
// all_true_converter — AND reduction over a bool vector
// ---------------------------------------------------------------------------
//
// Returns `true` iff every element is `true`.
// An empty vector yields `true` (vacuous conjunction).

struct all_true_converter : multi_value_converter<bool, bool> {
    [[nodiscard]] bool
    convert(const std::vector<bool>& values) const override {
        for (bool v : values) {
            if (!v) {
                return false;
            }
        }
        return true;
    }
};

/// Free-function helper — returns a `std::function` for use in lambdas /
/// multi_binding combiner slots without constructing the converter object.
[[nodiscard]] inline std::function<bool(const std::vector<bool>&)>
all_true() {
    return [](const std::vector<bool>& values) {
        for (bool v : values) {
            if (!v) {
                return false;
            }
        }
        return true;
    };
}

// ---------------------------------------------------------------------------
// any_true_converter — OR reduction over a bool vector
// ---------------------------------------------------------------------------
//
// Returns `true` iff at least one element is `true`.
// An empty vector yields `false` (vacuous disjunction).

struct any_true_converter : multi_value_converter<bool, bool> {
    [[nodiscard]] bool
    convert(const std::vector<bool>& values) const override {
        for (bool v : values) {
            if (v) {
                return true;
            }
        }
        return false;
    }
};

/// Free-function helper.
[[nodiscard]] inline std::function<bool(const std::vector<bool>&)>
any_true() {
    return [](const std::vector<bool>& values) {
        for (bool v : values) {
            if (v) {
                return true;
            }
        }
        return false;
    };
}

// ---------------------------------------------------------------------------
// concat_converter — join a string vector with a separator
// ---------------------------------------------------------------------------

struct concat_converter : multi_value_converter<std::string, std::string> {
    explicit concat_converter(std::string separator = "")
        : separator_{ std::move(separator) } {}

    [[nodiscard]] std::string
    convert(const std::vector<std::string>& values) const override {
        std::string result;
        bool        first = true;
        for (const auto& s : values) {
            if (!first) {
                result += separator_;
            }
            result += s;
            first = false;
        }
        return result;
    }

private:
    std::string separator_;
};

/// Free-function helper.
[[nodiscard]] inline
std::function<std::string(const std::vector<std::string>&)>
concat_strings(std::string separator = "") {
    return [sep = std::move(separator)](const std::vector<std::string>& values) {
        std::string result;
        bool        first = true;
        for (const auto& s : values) {
            if (!first) {
                result += sep;
            }
            result += s;
            first = false;
        }
        return result;
    };
}

// ---------------------------------------------------------------------------
// chain_converter — compose two single-value converters
// ---------------------------------------------------------------------------
//
// `chain_converter(c1, c2)` produces a `std::function<T(const S&)>` where
// c1 : S -> M  and  c2 : M -> T.  The output of c1 feeds c2.
//
// Both `c1` and `c2` are `std::function<…>` objects (the type that
// `binding<S,T>` takes in its converter slot, and that every free-function
// helper in converters.hpp returns).  No virtual dispatch, no heap beyond
// what `std::function` already uses.

template <class S, class M, class T>
[[nodiscard]] std::function<T(const S&)>
chain_converter(std::function<M(const S&)> c1,
                std::function<T(const M&)> c2) {
    return [c1 = std::move(c1), c2 = std::move(c2)](const S& value) -> T {
        M intermediate = c1(value);
        return c2(intermediate);
    };
}

} // namespace mpapp

#endif // MPAPP_BINDING_MULTI_VALUE_CONVERTER_HPP
