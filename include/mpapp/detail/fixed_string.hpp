// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. NTTP-friendly compile-time string wrapper.
//
// C++20 lets a literal class type be a non-type template parameter
// (NTTP), which is the trick that powers `mpapp::route<"home/details",
// ...>` and `mpapp::param<"id", int>`. This header defines the
// `fixed_string<N>` aggregate the rest of the routing layer uses.
//
//   template <fixed_string Path> struct route_marker { /* ... */ };
//
//   route_marker<"home/details"> r{};   // ok
//   auto v = r.path;                    // r.path is fixed_string<13>
//
// The constraints C++20 imposes on NTTP class types: structural type
// (all-public members, no user-declared dtor/copy/move/operator=,
// non-mutable members, no virtuals). `fixed_string` satisfies all of
// these by being a trivial aggregate.

#ifndef MPAPP_DETAIL_FIXED_STRING_HPP
#define MPAPP_DETAIL_FIXED_STRING_HPP

#include <cstddef>
#include <string_view>

namespace mpapp::detail {

template <std::size_t N>
struct fixed_string {
    // N includes the trailing '\0' captured by the array deduction
    // guide, so size() = N - 1.
    char data[N]{};

    constexpr fixed_string(const char (&s)[N]) {
        for (std::size_t i = 0; i < N; ++i) data[i] = s[i];
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return N - 1; }
    [[nodiscard]] constexpr const char* c_str() const noexcept { return data; }
    [[nodiscard]] constexpr std::string_view view() const noexcept {
        return std::string_view{data, N - 1};
    }

    // Same-length comparison — different-length specializations below.
    constexpr bool operator==(const fixed_string& o) const noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            if (data[i] != o.data[i]) return false;
        }
        return true;
    }
};

// Different-length comparison — needed for `route<"a">::path == route<"bb">::path`
// in constexpr contexts (returns false statically).
template <std::size_t N, std::size_t M>
constexpr bool operator==(const fixed_string<N>&, const fixed_string<M>&) noexcept {
    return false;
}

// Deduction guide so `fixed_string{"abc"}` and `fixed_string<"abc">`
// both work (the latter via the array deduction at the NTTP site).
template <std::size_t N>
fixed_string(const char (&)[N]) -> fixed_string<N>;

} // namespace mpapp::detail

#endif // MPAPP_DETAIL_FIXED_STRING_HPP
