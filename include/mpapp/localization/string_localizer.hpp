// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// string_localizer — a resx-equivalent in-memory localizer.
//
// Stores key/value pairs per culture (e.g. "en-US", "fr-FR").  The invariant
// fallback culture is the empty string ""; keys registered there are returned
// when neither the current culture nor an explicitly requested fallback
// culture carries the key.  If the key is absent everywhere it is returned
// verbatim as the value (MAUI ResourceManager behaviour).
//
// format() performs MAUI-style positional substitution: every occurrence of
// {N} in the localised string is replaced by the Nth argument converted to
// std::string via std::to_string (arithmetic) or direct construction
// (std::string / std::string_view).  Unknown indices and literal braces not
// matching the {N} pattern are passed through unchanged.
//
// culture_changed is an mpapp::signal<std::string> that fires with the new
// culture name whenever set_current_culture() actually changes the value.

#ifndef MPAPP_LOCALIZATION_STRING_LOCALIZER_HPP
#define MPAPP_LOCALIZATION_STRING_LOCALIZER_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../signal.hpp"

namespace mpapp {

// ---------------------------------------------------------------------------
// string_localizer
// ---------------------------------------------------------------------------

class string_localizer {
public:
    // The culture name used for invariant / fallback entries.
    static constexpr std::string_view invariant_culture = "";

    string_localizer() = default;

    string_localizer(const string_localizer&)            = delete;
    string_localizer& operator=(const string_localizer&) = delete;
    string_localizer(string_localizer&&)                 = delete;
    string_localizer& operator=(string_localizer&&)      = delete;

    ~string_localizer() = default;

    // -----------------------------------------------------------------------
    // Registration
    // -----------------------------------------------------------------------

    // Register (or replace) the string for `key` in `culture`.
    // Use the empty string "" as culture to register an invariant fallback.
    void add(std::string culture, std::string key, std::string value) {
        cultures_[std::move(culture)][std::move(key)] = std::move(value);
    }

    // -----------------------------------------------------------------------
    // Culture
    // -----------------------------------------------------------------------

    // Change the active culture.  Fires culture_changed if the name differs
    // from the current one.
    void set_current_culture(std::string name) {
        if (name == current_culture_) {
            return;
        }
        current_culture_ = std::move(name);
        culture_changed.emit(current_culture_);
    }

    [[nodiscard]] std::string current_culture() const {
        return current_culture_;
    }

    // -----------------------------------------------------------------------
    // Lookup
    // -----------------------------------------------------------------------

    // Resolve `key` in the current culture, then the invariant culture, then
    // return the key itself.
    [[nodiscard]] std::string get(std::string_view key) const {
        return resolve(key, current_culture_);
    }

    // Resolve `key` in `fallback_culture` first, then invariant, then key.
    [[nodiscard]] std::string get(std::string_view key,
                                  std::string_view fallback_culture) const {
        return resolve(key, std::string(fallback_culture));
    }

    // -----------------------------------------------------------------------
    // Formatting — {0}/{1}/... positional substitution
    // -----------------------------------------------------------------------

    template <class... A>
    [[nodiscard]] std::string format(std::string_view key, A&&... a) const {
        std::string pattern = get(key);
        std::vector<std::string> args;
        args.reserve(sizeof...(A));
        (args.push_back(to_arg_string(std::forward<A>(a))), ...);
        return substitute(pattern, args);
    }

    // -----------------------------------------------------------------------
    // Event
    // -----------------------------------------------------------------------

    mutable mpapp::signal<std::string> culture_changed;

private:
    // Culture name -> (key -> value)
    std::unordered_map<std::string,
                       std::unordered_map<std::string, std::string>> cultures_;
    std::string current_culture_;

    // Internal lookup helper: tries `preferred_culture`, then invariant "".
    [[nodiscard]] std::string resolve(std::string_view key,
                                     const std::string& preferred_culture) const {
        // 1. Preferred culture
        if (auto cit = cultures_.find(preferred_culture); cit != cultures_.end()) {
            const auto& table = cit->second;
            if (auto kit = table.find(std::string(key)); kit != table.end()) {
                return kit->second;
            }
        }
        // 2. Invariant fallback (empty culture)
        if (!preferred_culture.empty()) {
            auto inv = cultures_.find(std::string(invariant_culture));
            if (inv != cultures_.end()) {
                const auto& table = inv->second;
                if (auto kit = table.find(std::string(key)); kit != table.end()) {
                    return kit->second;
                }
            }
        }
        // 3. Key verbatim
        return std::string(key);
    }

    // {N} positional substitution.  Only replaces {N} where N is a valid
    // decimal integer index into `args`.  Other text is passed through.
    [[nodiscard]] static std::string substitute(
            const std::string& pattern,
            const std::vector<std::string>& args) {
        std::string result;
        result.reserve(pattern.size());

        std::size_t i = 0;
        while (i < pattern.size()) {
            if (pattern[i] == '{') {
                // Scan for the matching '}'
                std::size_t j = i + 1;
                while (j < pattern.size() && pattern[j] != '}') {
                    ++j;
                }
                if (j < pattern.size()) {
                    // We found '{...}'
                    std::string_view inner(&pattern[i + 1], j - i - 1);
                    // Try to parse inner as a non-negative integer index
                    std::size_t idx = 0;
                    bool valid = !inner.empty();
                    for (char c : inner) {
                        if (c < '0' || c > '9') { valid = false; break; }
                        idx = idx * 10u + static_cast<std::size_t>(c - '0');
                    }
                    if (valid && idx < args.size()) {
                        result += args[idx];
                    } else {
                        // Unknown placeholder — emit verbatim
                        result += '{';
                        result.append(inner);
                        result += '}';
                    }
                    i = j + 1;
                } else {
                    // No closing '}' — emit the lone '{' verbatim
                    result += '{';
                    ++i;
                }
            } else {
                result += pattern[i];
                ++i;
            }
        }
        return result;
    }

    // Conversion helpers: arithmetic -> std::to_string; string types direct.
    template <class T>
        requires std::is_arithmetic_v<std::remove_cvref_t<T>>
    [[nodiscard]] static std::string to_arg_string(T&& v) {
        return std::to_string(std::forward<T>(v));
    }

    [[nodiscard]] static std::string to_arg_string(std::string v) {
        return v;
    }

    [[nodiscard]] static std::string to_arg_string(std::string_view v) {
        return std::string(v);
    }

    [[nodiscard]] static std::string to_arg_string(const char* v) {
        return std::string(v);
    }
};

} // namespace mpapp

#endif // MPAPP_LOCALIZATION_STRING_LOCALIZER_HPP
