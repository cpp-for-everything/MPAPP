// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0009-behaviors-and-effects.md
//
// Additional CommunityToolkit-style neutral behaviors (surface-neutral;
// operate on signals and strings, not on native widgets):
//
//   mpapp::max_length_behavior
//     Validates or truncates a std::string to a configurable maximum length.
//     Exposes is_within_limit() and a value_truncated signal that fires with
//     the truncated string whenever a value exceeds the limit.
//
//   mpapp::regex_validation_behavior
//     Validates a std::string against a std::regex pattern. Exposes
//     is_valid() and a validity_changed signal that fires each time the
//     valid/invalid status flips.
//
//   mpapp::numeric_validation_behavior
//     Parses a numeric string and checks it against a [min, max] range.
//     Exposes is_valid() and a validity_changed signal that fires on
//     every valid/invalid flip.
//
// No macros in the public API (ADR-0002). Header-only. No wrapper
// components (ADR-0024).

#ifndef MPAPP_BEHAVIORS_BEHAVIORS_CTK2_HPP
#define MPAPP_BEHAVIORS_BEHAVIORS_CTK2_HPP

#include <cstddef>
#include <cstdint>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

#include "../signal.hpp"

namespace mpapp {

// ---------------------------------------------------------------------------
// max_length_behavior
//
// Validates and optionally truncates a std::string to a configurable maximum
// byte length. Call validate(text) to check / truncate the value.
//
//   is_within_limit() — true when the last validated value fits.
//   value_truncated   — fires with the truncated string whenever the supplied
//                       value exceeded the limit (fires even if the result
//                       stays equal across two consecutive over-limit calls).
//
// A max_length of 0 disables the cap (every string is within limit).
// ---------------------------------------------------------------------------
class max_length_behavior {
public:
    explicit max_length_behavior(std::size_t max_length = 0) noexcept
        : max_length_{ max_length }
    {}

    max_length_behavior(const max_length_behavior&)            = delete;
    max_length_behavior& operator=(const max_length_behavior&) = delete;
    max_length_behavior(max_length_behavior&&)                 = delete;
    max_length_behavior& operator=(max_length_behavior&&)      = delete;

    ~max_length_behavior() = default;

    // Validate `text` against max_length_. If the string exceeds the limit,
    // is_within_limit_ is set to false and value_truncated fires with the
    // substring [0, max_length_). If within (or no limit), is_within_limit_
    // is set to true.
    void validate(const std::string& text) {
        if (max_length_ > 0 && text.size() > max_length_) {
            is_within_limit_ = false;
            value_truncated.emit(text.substr(0, max_length_));
        } else {
            is_within_limit_ = true;
        }
    }

    [[nodiscard]] bool        is_within_limit() const noexcept { return is_within_limit_; }
    [[nodiscard]] std::size_t max_length()      const noexcept { return max_length_;       }

    // Fired with the truncated string (first max_length_ bytes) whenever the
    // input exceeds the limit.
    mpapp::signal<std::string> value_truncated{};

private:
    std::size_t max_length_      = 0;
    bool        is_within_limit_ = true;
};

// ---------------------------------------------------------------------------
// regex_validation_behavior
//
// Validates a std::string against a std::regex pattern. Call validate(text)
// to update state.
//
//   is_valid()       — true when the last validated value matched the pattern.
//   validity_changed — fires with the new validity state each time it flips.
//
// The regex is stored by value; copying or moving this behavior is disabled
// to match the rest of the CTK behaviors.
//
// Enum for the match mode (full-match vs. partial-match).
// ---------------------------------------------------------------------------
enum class regex_match_mode : std::uint8_t {
    full    = 0,   // std::regex_match  — entire string must match
    partial = 1,   // std::regex_search — any sub-sequence may match
};

[[nodiscard]] constexpr std::string_view to_string(regex_match_mode m) noexcept {
    switch (m) {
        case regex_match_mode::full:    return "full";
        case regex_match_mode::partial: return "partial";
    }
    return "unknown";
}

class regex_validation_behavior {
public:
    explicit regex_validation_behavior(
        std::regex        pattern,
        regex_match_mode  mode = regex_match_mode::full)
        : pattern_{ std::move(pattern) }
        , mode_{ mode }
    {}

    regex_validation_behavior(const regex_validation_behavior&)            = delete;
    regex_validation_behavior& operator=(const regex_validation_behavior&) = delete;
    regex_validation_behavior(regex_validation_behavior&&)                 = delete;
    regex_validation_behavior& operator=(regex_validation_behavior&&)      = delete;

    ~regex_validation_behavior() = default;

    // Validate `text` against pattern_. Updates is_valid_ and fires
    // validity_changed iff the valid/invalid status flips.
    void validate(const std::string& text) {
        const bool next = (mode_ == regex_match_mode::full)
                        ? std::regex_match(text, pattern_)
                        : std::regex_search(text, pattern_);
        if (next != is_valid_) {
            is_valid_ = next;
            validity_changed.emit(is_valid_);
        }
    }

    [[nodiscard]] bool             is_valid() const noexcept { return is_valid_; }
    [[nodiscard]] regex_match_mode mode()     const noexcept { return mode_;     }

    // Fired with the new validity state each time it flips.
    mpapp::signal<bool> validity_changed{};

private:
    std::regex       pattern_;
    regex_match_mode mode_     = regex_match_mode::full;
    bool             is_valid_ = false;
};

// ---------------------------------------------------------------------------
// numeric_validation_behavior
//
// Parses a std::string as a double and checks whether the value lies within
// [min_value_, max_value_]. Call validate(text) to update state.
//
//   is_valid()       — true when the last validated text was parseable and
//                      within the configured range.
//   validity_changed — fires with the new validity state each time it flips.
//
// Range semantics:
//   - min == max == 0.0  =>  no range check; any parseable number is valid.
//   - Otherwise          =>  min_value_ <= parsed <= max_value_ must hold.
//
// Parse failure (non-numeric text, extra trailing chars) always yields false.
// ---------------------------------------------------------------------------
class numeric_validation_behavior {
public:
    explicit numeric_validation_behavior(double min_value = 0.0,
                                         double max_value = 0.0) noexcept
        : min_value_{ min_value }
        , max_value_{ max_value }
    {}

    numeric_validation_behavior(const numeric_validation_behavior&)            = delete;
    numeric_validation_behavior& operator=(const numeric_validation_behavior&) = delete;
    numeric_validation_behavior(numeric_validation_behavior&&)                 = delete;
    numeric_validation_behavior& operator=(numeric_validation_behavior&&)      = delete;

    ~numeric_validation_behavior() = default;

    // Validate `text`. Updates is_valid_ and fires validity_changed on flip.
    void validate(const std::string& text) {
        const bool next = compute_valid(text);
        if (next != is_valid_) {
            is_valid_ = next;
            validity_changed.emit(is_valid_);
        }
    }

    [[nodiscard]] bool   is_valid()   const noexcept { return is_valid_;   }
    [[nodiscard]] double min_value()  const noexcept { return min_value_;  }
    [[nodiscard]] double max_value()  const noexcept { return max_value_;  }

    // Fired with the new validity state each time it flips.
    mpapp::signal<bool> validity_changed{};

private:
    [[nodiscard]] bool compute_valid(const std::string& text) const {
        if (text.empty()) {
            return false;
        }
        std::size_t pos = 0;
        double parsed = 0.0;
        try {
            parsed = std::stod(text, &pos);
        } catch (const std::invalid_argument&) {
            return false;
        } catch (const std::out_of_range&) {
            return false;
        }
        // Reject trailing non-whitespace characters.
        if (pos != text.size()) {
            return false;
        }
        // No range constraint when both bounds are zero.
        if (min_value_ == 0.0 && max_value_ == 0.0) {
            return true;
        }
        return parsed >= min_value_ && parsed <= max_value_;
    }

    double min_value_ = 0.0;
    double max_value_ = 0.0;
    bool   is_valid_  = false;
};

} // namespace mpapp

#endif // MPAPP_BEHAVIORS_BEHAVIORS_CTK2_HPP
