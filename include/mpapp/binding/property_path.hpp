// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// `mpapp::property_path<Root, Leaf>` — a typed, null-safe nested
// property-path accessor. This is what MAUI's string binding paths
// ("A.B.C") compile down to in a type-safe, macro-free form (Rule 1 /
// ADR-0002, ADR-0009: public API is template wrappers only).
//
// A path is built from a chain of *links*. Each intermediate link knows
// how to walk from one object to a pointer-to-the-next object (its
// getter); the terminal link knows how to reach a reference to the leaf
// value (for both read and write). Getters return raw pointers, so a
// broken intermediate (e.g. a null `unique_ptr`/`shared_ptr` member, or a
// getter that legitimately has no value) is modelled as `nullptr`. When
// any link in the chain yields `nullptr`, `get()` returns `std::nullopt`
// and `set()` returns `false` — the path fails gracefully instead of
// dereferencing null.
//
// property_path is platform-NEUTRAL infrastructure and owns no native
// widget (ADR-0024). It is header-only.
//
// Construction is via the free `make_property_path(...)` helper plus the
// `path_link(...)` / `path_leaf(...)` link factories, so the user never
// has to spell the (otherwise recursive) accessor type by hand:
//
//   struct Leaf { int n = 0; };
//   struct Mid  { Leaf* leaf = nullptr; };
//   struct Root { Mid*  mid  = nullptr; };
//
//   auto p = make_property_path(
//       path_link([](Root& r){ return r.mid; }),    // Root -> Mid*
//       path_link([](Mid& m){ return m.leaf; }),     // Mid  -> Leaf*
//       path_leaf([](Leaf& l) -> int* { return &l.n; }));
//
//   std::optional<int> v = p.get(root);   // nullopt if any link is null
//   bool ok = p.set(root, 42);            // false if any link is null

#ifndef MPAPP_BINDING_PROPERTY_PATH_HPP
#define MPAPP_BINDING_PROPERTY_PATH_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace mpapp {

// --- callable trait: deduce the single argument + return type of a
// lambda / function object via its operator(). -----------------------------
template <class T>
struct callable_traits : callable_traits<decltype(&T::operator())> {};

template <class C, class R, class A>
struct callable_traits<R (C::*)(A) const> {
    using arg_type    = A;
    using return_type = R;
};
template <class C, class R, class A>
struct callable_traits<R (C::*)(A)> {
    using arg_type    = A;
    using return_type = R;
};

// --- a single intermediate hop: maps `From&` -> `To*` (nullable). ----------
// A null `From*` or null getter short-circuits to `nullptr`, so chained
// steps propagate a broken link without ever dereferencing null.
template <class From, class To>
class path_step {
public:
    using from_type = From;
    using to_type   = To;
    using getter_fn = std::function<To*(From&)>;

    explicit path_step(getter_fn getter) : getter_{ std::move(getter) } {}

    [[nodiscard]] To* step(From* from) const {
        if (from == nullptr || !getter_) {
            return nullptr;
        }
        return getter_(*from);
    }

private:
    getter_fn getter_;
};

// --- the terminal hop: maps `From&` -> `Leaf*` (address of the value). -----
// Null = the leaf is unreachable from this object. The same address is
// used for both reading (`*leaf`) and writing (`*leaf = value`).
template <class From, class Leaf>
class path_terminal {
public:
    using from_type = From;
    using leaf_type = Leaf;
    using getter_fn = std::function<Leaf*(From&)>;

    explicit path_terminal(getter_fn getter) : getter_{ std::move(getter) } {}

    [[nodiscard]] Leaf* address(From* from) const {
        if (from == nullptr || !getter_) {
            return nullptr;
        }
        return getter_(*from);
    }

private:
    getter_fn getter_;
};

// --- link factories: callers pass lambdas, not types. ----------------------

// Intermediate link from a callable with signature `To*(From&)`.
template <class Fn>
[[nodiscard]] auto path_link(Fn fn) {
    using From = std::remove_cvref_t<typename callable_traits<Fn>::arg_type>;
    using Ret  = typename callable_traits<Fn>::return_type;  // To*
    using To   = std::remove_pointer_t<Ret>;
    return path_step<From, To>{ std::function<To*(From&)>{ std::move(fn) } };
}

// Terminal link from a callable with signature `Leaf*(From&)`.
template <class Fn>
[[nodiscard]] auto path_leaf(Fn fn) {
    using From = std::remove_cvref_t<typename callable_traits<Fn>::arg_type>;
    using Ret  = typename callable_traits<Fn>::return_type;  // Leaf*
    using Leaf = std::remove_pointer_t<Ret>;
    return path_terminal<From, Leaf>{
        std::function<Leaf*(From&)>{ std::move(fn) }
    };
}

// --- the composed path. ----------------------------------------------------
// `Root` is the type the path is rooted at, `Leaf` is the value type, and
// `Steps...` are the (possibly empty) intermediate `path_step`s. The
// terminal's `From` is the previous link's `To` (or `Root` when there are
// no intermediate steps).
template <class Root, class Leaf, class TerminalFrom, class... Steps>
class property_path {
public:
    using root_type = Root;
    using leaf_type = Leaf;

    explicit property_path(std::tuple<Steps...>                  steps,
                           path_terminal<TerminalFrom, Leaf>     terminal)
        : steps_{ std::move(steps) }, terminal_{ std::move(terminal) } {}

    // Read: nullopt if any link (including the leaf) resolves to null.
    [[nodiscard]] std::optional<Leaf> get(Root& root) const {
        Leaf* leaf = resolve(root);
        if (leaf == nullptr) {
            return std::nullopt;
        }
        return std::optional<Leaf>{ *leaf };
    }

    // Write: false (no-op) if any link is null, true on success.
    [[nodiscard]] bool set(Root& root, const Leaf& value) const {
        Leaf* leaf = resolve(root);
        if (leaf == nullptr) {
            return false;
        }
        *leaf = value;
        return true;
    }

private:
    [[nodiscard]] Leaf* resolve(Root& root) const { return walk<0>(&root); }

    // Apply step N, then recurse; once steps are exhausted apply the
    // terminal. Every hop null-short-circuits internally.
    template <std::size_t N, class Cur>
    [[nodiscard]] Leaf* walk(Cur* cur) const {
        if constexpr (N == sizeof...(Steps)) {
            return terminal_.address(cur);
        } else {
            const auto& s = std::get<N>(steps_);
            return walk<N + 1>(s.step(cur));
        }
    }

    std::tuple<Steps...>              steps_;
    path_terminal<TerminalFrom, Leaf> terminal_;
};

// --- builder: assemble steps + terminal into a property_path. --------------
//
// The Root is the first step's `From`, or — when there are no intermediate
// steps — the terminal's `From`. The terminal's `From` must equal the last
// step's `To`; this is enforced naturally by the link `From`/`To` types.

// Internal accumulator (declared before use): collect `path_step`s into
// `acc`, recurse on the remaining links, and close out on the terminal.
template <class Root, class... Acc, class TermFrom, class Leaf>
[[nodiscard]] auto detail_make_path(std::tuple<Acc...>            acc,
                                    path_terminal<TermFrom, Leaf> terminal) {
    return property_path<Root, Leaf, TermFrom, Acc...>{ std::move(acc),
                                                        std::move(terminal) };
}

template <class Root, class... Acc, class StepFrom, class StepTo, class... Rest>
[[nodiscard]] auto detail_make_path(std::tuple<Acc...>          acc,
                                    path_step<StepFrom, StepTo> next,
                                    Rest... rest) {
    return detail_make_path<Root>(
        std::tuple_cat(std::move(acc),
                       std::tuple<path_step<StepFrom, StepTo>>{
                           std::move(next) }),
        std::move(rest)...);
}

// No intermediate steps: path is just Root -> Leaf.
template <class From, class Leaf>
[[nodiscard]] auto make_property_path(path_terminal<From, Leaf> terminal) {
    return property_path<From, Leaf, From>{ std::tuple<>{},
                                            std::move(terminal) };
}

// One or more intermediate steps followed by a terminal. Root is the first
// step's `From`. Peel links into an accumulator tuple, terminating on the
// leaf.
template <class FirstFrom, class FirstTo, class... Rest>
[[nodiscard]] auto make_property_path(path_step<FirstFrom, FirstTo> first,
                                      Rest... rest) {
    return detail_make_path<FirstFrom>(
        std::tuple<path_step<FirstFrom, FirstTo>>{ std::move(first) },
        std::move(rest)...);
}

} // namespace mpapp

#endif // MPAPP_BINDING_PROPERTY_PATH_HPP
