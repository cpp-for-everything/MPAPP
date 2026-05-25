// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0005-resource-dictionaries-and-styling.md
//
// `mpapp::resource_recorder` — mock-only wrapper around a
// `resource_dictionary` that records every put/remove + lookup as a
// `call_record` so tests can assert exactly which keys were touched
// and in what order. Mirrors the mock `image_loader` from RFC-0004:
// thin handler-side recording around a configuration type, no
// per-platform real-equivalent required.

#ifndef MPAPP_HANDLERS_MOCK_RESOURCE_RECORDER_HPP
#define MPAPP_HANDLERS_MOCK_RESOURCE_RECORDER_HPP

#include <optional>
#include <string>

#include "../../resources/resource_dictionary.hpp"
#include "handler_base.hpp"

namespace mpapp {

class resource_recorder : public mock_handler_base {
public:
    explicit resource_recorder(resource_dictionary& d) : dict_{ &d } {
        d.changed.subscribe(changed_slot_, changed_cb_);
        d.composition_changed.subscribe(composition_slot_, composition_cb_);
    }

    resource_recorder(const resource_recorder&)            = delete;
    resource_recorder& operator=(const resource_recorder&) = delete;
    resource_recorder(resource_recorder&&)                 = delete;
    resource_recorder& operator=(resource_recorder&&)      = delete;

    // Lookup wrapper. Records "lookup.hit=<key>" or "lookup.miss=<key>"
    // depending on whether the dictionary chain produced a `T`-typed
    // value. Returns the same `std::optional<T>` the underlying
    // dictionary returned so tests can chain assertions.
    template <class T>
    std::optional<T> try_get(const std::string& key) {
        auto result = dict_->try_get<T>(key);
        record_change(result.has_value() ? "lookup.hit" : "lookup.miss", key);
        return result;
    }

    [[nodiscard]] resource_dictionary&       dict() noexcept       { return *dict_; }
    [[nodiscard]] const resource_dictionary& dict() const noexcept { return *dict_; }

private:
    // Callbacks are stored as members so their addresses are stable
    // across the recorder's lifetime — the intrusive slot lists in
    // `signal` capture the callable by raw pointer (see signal.hpp).
    struct changed_cb_t {
        resource_recorder* self;
        void operator()(const resource_dictionary::change& c) const {
            self->record(c.new_value ? "put" : "remove", std::string{ c.key });
        }
    };

    struct composition_cb_t {
        resource_recorder* self;
        void operator()() const { self->record_event("composition_changed"); }
    };

    resource_dictionary*                                dict_;
    changed_cb_t                                        changed_cb_{ this };
    composition_cb_t                                    composition_cb_{ this };
    signal_slot<const resource_dictionary::change&>     changed_slot_{};
    signal_slot<>                                       composition_slot_{};
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_RESOURCE_RECORDER_HPP
