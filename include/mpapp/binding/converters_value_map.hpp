// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// CTK-style value-mapping converters (batch 2).  Complements
// converters.hpp (bool/visibility/string-format) with converters that
// treat a source value as a lookup key:
//
//   bool_to_object<T>     — maps true/false to arbitrary T values
//   enum_to_bool<E>       — true when an enum value matches a sentinel
//   int_to_bool           — false when the int is 0, true otherwise
//   index_to_array_item<T>— picks an element from a vector by index
//   item_to_index<T>      — reverse: find an element's position
//
// Each type is both a `value_converter<S,T>` object (for resource
// dictionaries) and a self-contained callable, so it can be handed
// directly to `binding<S,T>`'s converter slot.  No macros; platform-
// neutral; header-only.

#ifndef MPAPP_BINDING_CONVERTERS_VALUE_MAP_HPP
#define MPAPP_BINDING_CONVERTERS_VALUE_MAP_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "binding.hpp"   // for mpapp::value_converter

namespace mpapp {

// ---- bool -> T (CommunityToolkit BoolToObjectConverter) -------------------
//
// Maps true to `true_value` and false to `false_value`.
// convert_back: equality-compare target against `true_value`.

template <class T>
struct bool_to_object_converter : value_converter<bool, T> {
    T true_value_{};
    T false_value_{};

    bool_to_object_converter() = default;
    bool_to_object_converter(T tv, T fv)
        : true_value_{ std::move(tv) }
        , false_value_{ std::move(fv) } {}

    [[nodiscard]] T convert(const bool& source) const override {
        return source ? true_value_ : false_value_;
    }
    [[nodiscard]] bool convert_back(const T& target) const override {
        return target == true_value_;
    }

    // Callable operator so the object can be passed as a converter fn.
    [[nodiscard]] T operator()(bool source) const {
        return source ? true_value_ : false_value_;
    }
};

// ---- enum -> bool (CommunityToolkit EnumToBoolConverter) ------------------
//
// Returns true when the source enum value equals `match_`.
// convert_back: returns `match_` when target is true, otherwise a
// default-constructed E (undefined in MAUI too; callers use one_way).

template <class E>
struct enum_to_bool_converter : value_converter<E, bool> {
    E match_{};

    enum_to_bool_converter() = default;
    explicit enum_to_bool_converter(E match) : match_{ match } {}

    [[nodiscard]] bool convert(const E& source) const override {
        return source == match_;
    }
    [[nodiscard]] E convert_back(const bool& target) const override {
        return target ? match_ : E{};
    }

    [[nodiscard]] bool operator()(E source) const {
        return source == match_;
    }
};

// ---- int -> bool (CommunityToolkit IntToBoolConverter) --------------------
//
// Zero maps to false; any other int maps to true (truthiness semantics).
// convert_back: true -> 1, false -> 0.

struct int_to_bool_converter : value_converter<int, bool> {
    [[nodiscard]] bool convert(const int& source) const override {
        return source != 0;
    }
    [[nodiscard]] int convert_back(const bool& target) const override {
        return target ? 1 : 0;
    }

    [[nodiscard]] bool operator()(int source) const {
        return source != 0;
    }
};

// ---- int -> optional<T> (CommunityToolkit IndexToArrayItemConverter) ------
//
// Returns the element at position `index` in `items_`, or std::nullopt
// when the index is out of range (including negative).
// convert_back: returns the position of `target` in `items_`, or -1.

template <class T>
struct index_to_array_item_converter : value_converter<int, std::optional<T>> {
    std::vector<T> items_{};

    index_to_array_item_converter() = default;
    explicit index_to_array_item_converter(std::vector<T> items)
        : items_{ std::move(items) } {}

    [[nodiscard]] std::optional<T> convert(const int& source) const override {
        if (source < 0 || static_cast<std::size_t>(source) >= items_.size()) {
            return std::nullopt;
        }
        return items_[static_cast<std::size_t>(source)];
    }

    // convert_back: find item in vector, return its index or -1.
    [[nodiscard]] int convert_back(const std::optional<T>& target) const override {
        if (!target.has_value()) {
            return -1;
        }
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (items_[i] == *target) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    [[nodiscard]] std::optional<T> operator()(int source) const {
        return convert(source);
    }
};

// ---- T -> int (CommunityToolkit ItemToIndexConverter) ---------------------
//
// Reverse of index_to_array_item: given a value, returns its zero-based
// index in `items_`, or -1 when not found.
// convert_back: delegates to index_to_array_item logic.

template <class T>
struct item_to_index_converter : value_converter<T, int> {
    std::vector<T> items_{};

    item_to_index_converter() = default;
    explicit item_to_index_converter(std::vector<T> items)
        : items_{ std::move(items) } {}

    [[nodiscard]] int convert(const T& source) const override {
        for (std::size_t i = 0; i < items_.size(); ++i) {
            if (items_[i] == source) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // convert_back: index -> item (std::optional not used here; out-of-range
    // returns a default-constructed T, consistent with MAUI semantics).
    [[nodiscard]] T convert_back(const int& target) const override {
        if (target < 0 || static_cast<std::size_t>(target) >= items_.size()) {
            return T{};
        }
        return items_[static_cast<std::size_t>(target)];
    }

    [[nodiscard]] int operator()(const T& source) const {
        return convert(source);
    }
};

} // namespace mpapp

#endif // MPAPP_BINDING_CONVERTERS_VALUE_MAP_HPP
