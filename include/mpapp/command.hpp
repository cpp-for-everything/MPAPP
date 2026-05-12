// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/20_ADRs/ADR-0009-public-api-template-wrappers-only.md
//
// Command<Args...> — empty tag type used as an unnamed default-valued
// parameter on a member function to declare the function as a bindable
// command:
//
//   void increment(Command<> = {});
//   void rename(std::string new_name, Command<std::string> = {});
//
// The framework recognises the type via `is_command_tag` and uses
// `command_traits<...>` to extract the argument list.

#ifndef MPAPP_COMMAND_HPP
#define MPAPP_COMMAND_HPP

#include <cstddef>
#include <type_traits>

namespace mpapp {

template <class... Args>
struct Command {};

namespace detail {

template <class>
struct is_command_tag_impl : std::false_type {};

template <class... Args>
struct is_command_tag_impl<Command<Args...>> : std::true_type {};

// Inspect the last parameter of a parameter pack — the special-case for
// an empty pack must yield `false`, not a hard substitution failure.
template <class... Ps>
constexpr bool pack_ends_with_command = false;

template <class P0, class... Rest>
constexpr bool pack_ends_with_command<P0, Rest...> = []{
    if constexpr (sizeof...(Rest) == 0) {
        return is_command_tag_impl<std::remove_cvref_t<P0>>::value;
    } else {
        return pack_ends_with_command<Rest...>;
    }
}();

// Primary template: not a member-function pointer → false.
template <class T>
constexpr bool last_param_is_command = false;

template <class C, class R, class... Ps>
constexpr bool last_param_is_command<R (C::*)(Ps...)> =
    pack_ends_with_command<Ps...>;

template <class C, class R, class... Ps>
constexpr bool last_param_is_command<R (C::*)(Ps...) const> =
    pack_ends_with_command<Ps...>;

template <class C, class R, class... Ps>
constexpr bool last_param_is_command<R (C::*)(Ps...) noexcept> =
    pack_ends_with_command<Ps...>;

template <class C, class R, class... Ps>
constexpr bool last_param_is_command<R (C::*)(Ps...) const noexcept> =
    pack_ends_with_command<Ps...>;

} // namespace detail

// True iff T is some `Command<...>` instantiation (after stripping cv-ref).
template <class T>
concept is_command_tag =
    detail::is_command_tag_impl<std::remove_cvref_t<T>>::value;

// True iff the supplied non-type template argument is a pointer-to-member
// function whose final parameter is a `Command<...>` tag.
template <auto Method>
concept is_command_method =
    detail::last_param_is_command<decltype(Method)>;

template <class C>
struct command_traits;

template <class... Args>
struct command_traits<Command<Args...>> {
    static constexpr std::size_t count = sizeof...(Args);
};

} // namespace mpapp

#endif // MPAPP_COMMAND_HPP
