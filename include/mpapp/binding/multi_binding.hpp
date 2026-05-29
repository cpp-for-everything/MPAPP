// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0007-data-binding.md
//
// `mpapp::multi_binding<T, Ss...>` — combine N source Observables into a
// single target `Observable<T>` through a combiner callable. Counterpart
// to MAUI's `MultiBinding` + `IMultiValueConverter`. One-way only
// (sources -> target); recomputes the target whenever ANY source changes.
//
// Composes with the reactive primitives exactly like `binding<S,T>`:
// owns one `signal_slot` per source, subscribes to each source's
// `changed` signal. No macros (ADR-0009). Platform-neutral.

#ifndef MPAPP_BINDING_MULTI_BINDING_HPP
#define MPAPP_BINDING_MULTI_BINDING_HPP

#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

#include "../observable.hpp"
#include "../signal.hpp"

namespace mpapp {

template <class T, class... Ss>
class multi_binding {
public:
    using combiner_fn = std::function<T(const Ss&...)>;

    // Recomputes `target` from `combine(sources...)` immediately, then on
    // every subsequent change to any source.
    explicit multi_binding(Observable<T>& target, combiner_fn combine,
                           Observable<Ss>&... sources)
        : target_{ &target }
        , combine_{ std::move(combine) }
        , sources_{ &sources... }
        , cbs_{ recompute_cb<Ss>{ this }... } {
        recompute();
        subscribe_all(std::index_sequence_for<Ss...>{});
    }

    multi_binding(const multi_binding&)            = delete;
    multi_binding& operator=(const multi_binding&) = delete;
    multi_binding(multi_binding&&)                 = delete;  // owns slots
    multi_binding& operator=(multi_binding&&)      = delete;

    ~multi_binding() = default;

private:
    void recompute() {
        std::apply(
            [this](auto*... srcs) { target_->set(combine_(srcs->get()...)); },
            sources_);
    }

    template <std::size_t... I>
    void subscribe_all(std::index_sequence<I...>) {
        (subscribe_one<I>(), ...);
    }

    template <std::size_t I>
    void subscribe_one() {
        std::get<I>(sources_)->changed.subscribe(std::get<I>(slots_),
                                                 std::get<I>(cbs_));
    }

    template <class U>
    struct recompute_cb {
        multi_binding* self;
        void operator()(const U&) const { self->recompute(); }
    };

    Observable<T>*                        target_;
    combiner_fn                           combine_;
    std::tuple<Observable<Ss>*...>        sources_;
    std::tuple<recompute_cb<Ss>...>       cbs_;
    std::tuple<signal_slot<const Ss&>...> slots_{};
};

} // namespace mpapp

#endif // MPAPP_BINDING_MULTI_BINDING_HPP
