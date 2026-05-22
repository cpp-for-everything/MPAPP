// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Compile-time route table for `mpapp::shell` per
// [[ADR-0016-shell-compile-time-routes]].
//
// Apps declare their routes in one `route_table` value:
//
//   inline constexpr auto routes = mpapp::route_table{
//       mpapp::route<"home",          home_page>{},
//       mpapp::route<"home/details",  details_page, mpapp::param<"id", int>>{},
//       mpapp::route<"settings",      settings_page>{},
//   };
//
//   // C++ side — compile-time checked:
//   shell.go_to<"home/details", &routes>(42);     // ok
//   shell.go_to<"home/detial",  &routes>(42);     // compile error: not in table
//   shell.go_to<"home/details", &routes>("42");   // compile error: int expected, got const char*
//
// The XAML compiler still emits string-based `Shell.GoToAsync("//home/details?id=42")`
// calls — those resolve at runtime through the same `route_table`'s
// `runtime_has(name)` helper. The compile-time path is the C++ surface;
// the runtime path is the XAML escape hatch. Both go through
// `shell::go_to(std::string_view uri)` for the actual navigation,
// so the side-effects on `current_route` / `current_tab_index` /
// `navigated` are identical.
//
// Argument → URI stringification uses `to_route_string(value)`
// (ADL-customizable). Builtins for int/long/double/bool/string ship
// inline below; user types extend via:
//
//   namespace my_app {
//       std::string to_route_string(const my_id& v) { return v.repr(); }
//   }

#ifndef MPAPP_ROUTE_HPP
#define MPAPP_ROUTE_HPP

#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "detail/fixed_string.hpp"

namespace mpapp {

using detail::fixed_string;

// ---- Builtin to_route_string overloads -----------------------------------
//
// Lives in `mpapp` namespace; user code adds its own ADL overloads in
// the user type's namespace. The lookup happens via unqualified
// `to_route_string(v)` inside `route_table::build_uri`.

inline std::string to_route_string(int v)              { return std::to_string(v); }
inline std::string to_route_string(long v)             { return std::to_string(v); }
inline std::string to_route_string(long long v)        { return std::to_string(v); }
inline std::string to_route_string(unsigned v)         { return std::to_string(v); }
inline std::string to_route_string(unsigned long v)    { return std::to_string(v); }
inline std::string to_route_string(double v)           { return std::to_string(v); }
inline std::string to_route_string(bool v)             { return v ? "true" : "false"; }
inline std::string to_route_string(const std::string& v) { return v; }
inline std::string to_route_string(std::string_view v) { return std::string{v}; }
inline std::string to_route_string(const char* v)      { return std::string{v}; }

// ---- Route descriptors ---------------------------------------------------

// `param<"name", T>` describes a typed query parameter on a route.
// The route's go_to call expects an argument of type T for each param.
template <fixed_string Name, class T>
struct param {
    static constexpr auto name = Name;
    using value_t              = T;
};

// `route<"path", PageType, Params...>` is one row of the route_table.
// `Path` is the route string sans the leading `//`. `PageType` is the
// page class that should mount when this route is navigated; mock
// builds don't instantiate it but the type is carried through so
// future real-handler work has it available.
template <fixed_string Path, class PageType, class... Params>
struct route {
    static constexpr auto path = Path;
    using page_t               = PageType;
    using params_t             = std::tuple<Params...>;
    static constexpr std::size_t param_count = sizeof...(Params);
};

namespace detail {

// Index-based search for the route matching `Path` inside the tuple
// `RoutesTup = std::tuple<R0, R1, ...>`. We use index_sequence rather
// than recursive partial specializations so the static_assert hangs
// off a value-dependent expression (the path index) and produces a
// clean diagnostic.
template <fixed_string Path, class RoutesTup>
struct route_finder;

template <fixed_string Path, class... Routes>
struct route_finder<Path, std::tuple<Routes...>> {
    template <std::size_t I>
    static constexpr bool matches() noexcept {
        using R = std::tuple_element_t<I, std::tuple<Routes...>>;
        return R::path == Path;
    }

    template <std::size_t... I>
    static constexpr std::size_t find_index(std::index_sequence<I...>) noexcept {
        std::size_t idx = sizeof...(Routes); // sentinel = not found
        ((matches<I>() ? (idx = I) : 0), ...);
        return idx;
    }

    static constexpr std::size_t index =
        find_index(std::index_sequence_for<Routes...>{});

    static_assert(index < sizeof...(Routes),
                  "route_table: route not found in table");

    using type = std::tuple_element_t<index, std::tuple<Routes...>>;
};

template <class Param, class Arg>
void route_append_one_arg(std::string& out, bool& first, const Arg& a) {
    static_assert(std::is_convertible_v<Arg, typename Param::value_t>,
                  "route_table: argument type doesn't match param's declared type");
    out += first ? '?' : '&';
    first = false;
    out.append(Param::name.c_str(), Param::name.size());
    out += '=';
    // ADL lookup of to_route_string; falls back to the inline
    // builtins in mpapp:: above for primitive types.
    using ::mpapp::to_route_string;
    out += to_route_string(static_cast<typename Param::value_t>(a));
}

template <class ParamsTuple, std::size_t... I, class... Args>
void route_append_args(std::string& out,
                       std::index_sequence<I...>,
                       const Args&... args) {
    // [[maybe_unused]] handles the empty-Args... case where the fold
    // expression collapses to nothing and `first` is never read —
    // GCC's -Wunused-but-set-variable otherwise fires.
    [[maybe_unused]] bool first = true;
    ((route_append_one_arg<std::tuple_element_t<I, ParamsTuple>>(out, first, args)), ...);
}

} // namespace detail

// ---- The table itself ----------------------------------------------------

template <class... Routes>
struct route_table {
    using routes_t = std::tuple<Routes...>;
    static constexpr std::size_t size = sizeof...(Routes);

    constexpr route_table(Routes... /*rs*/) noexcept {}

    // Compile-time has-route check. The C++ navigation path uses this
    // in a static_assert to reject typos.
    template <fixed_string Path>
    static constexpr bool has() noexcept {
        return (... || (Routes::path == Path));
    }

    // Compile-time lookup of the matching route descriptor itself.
    // Has nested `params_t`, `page_t`, `path`. Static-asserts at
    // instantiation if no route matches.
    template <fixed_string Path>
    using route_for = typename detail::route_finder<Path, routes_t>::type;

    // Convenience aliases.
    template <fixed_string Path>
    using params_for = typename route_for<Path>::params_t;

    template <fixed_string Path>
    using page_for = typename route_for<Path>::page_t;

    // Runtime "is this route known?" — used by the XAML / string path
    // (shell::go_to(std::string_view)) to validate routes that came in
    // as strings. Walks the route list and compares string_views.
    [[nodiscard]] static bool runtime_has(std::string_view name) noexcept {
        return runtime_has_impl(name, std::index_sequence_for<Routes...>{});
    }

    // Build a "//path?p1=v1&p2=v2" URI from a route + typed arguments.
    // The argument count and types must match the route's params at
    // call time (the shell::go_to template asserts this at compile
    // time before calling).
    template <fixed_string Path, class... Args>
    [[nodiscard]] static std::string build_uri(const Args&... args) {
        using route_t  = route_for<Path>;
        using params_t = typename route_t::params_t;
        std::string out;
        out.reserve(2 + Path.size() + 16);
        out += "//";
        out.append(Path.c_str(), Path.size());
        detail::route_append_args<params_t>(out,
                                            std::index_sequence_for<Args...>{},
                                            args...);
        return out;
    }

private:
    template <std::size_t... I>
    [[nodiscard]] static bool runtime_has_impl(std::string_view name,
                                               std::index_sequence<I...>) noexcept {
        return (... || (std::tuple_element_t<I, std::tuple<Routes...>>::path.view() == name));
    }
};

template <class... Routes>
route_table(Routes...) -> route_table<Routes...>;

} // namespace mpapp

#endif // MPAPP_ROUTE_HPP
