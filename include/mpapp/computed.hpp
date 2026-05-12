// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// Computed<&Member, ...> — empty tag type used as an unnamed default-valued
// parameter on a member function to declare it as a computed property whose
// dependencies are the listed pointer-to-members.
//
//   auto display(Computed<&VM::a, &VM::b> = {}) const;
//
// The framework recognises the type via the `is_computed_tag` concept and
// extracts the dependency list with `computed_traits<...>`.

#ifndef MPAPP_COMPUTED_HPP
#define MPAPP_COMPUTED_HPP

#include <cstddef>
#include <type_traits>

namespace mpapp {

template <auto... DepPtrs>
struct Computed {};

namespace detail {

template <class>
struct is_computed_tag_impl : std::false_type {};

template <auto... Ptrs>
struct is_computed_tag_impl<Computed<Ptrs...>> : std::true_type {};

} // namespace detail

// True iff T is some `Computed<...>` instantiation (after stripping cv-ref).
template <class T>
concept is_computed_tag =
    detail::is_computed_tag_impl<std::remove_cvref_t<T>>::value;

template <class C>
struct computed_traits;

template <auto... Ptrs>
struct computed_traits<Computed<Ptrs...>> {
    static constexpr std::size_t count = sizeof...(Ptrs);
};

} // namespace mpapp

#endif // MPAPP_COMPUTED_HPP
