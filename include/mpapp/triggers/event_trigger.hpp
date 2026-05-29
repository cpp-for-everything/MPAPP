// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0008-triggers.md
//
// `mpapp::event_trigger<Args...>` — runs an action whenever a source
// `signal<Args...>` fires. Counterpart to MAUI's EventTrigger +
// TriggerAction. The action is a plain callable (the TriggerAction
// analog); it receives the signal's arguments. Owns its slot; no
// macros; platform-neutral.

#ifndef MPAPP_TRIGGERS_EVENT_TRIGGER_HPP
#define MPAPP_TRIGGERS_EVENT_TRIGGER_HPP

#include <functional>
#include <utility>

#include "../signal.hpp"

namespace mpapp {

template <class... Args>
class event_trigger {
public:
    using action_fn = std::function<void(Args...)>;

    event_trigger(signal<Args...>& source, action_fn action)
        : action_{ std::move(action) } {
        source.subscribe(slot_, cb_);
    }

    event_trigger(const event_trigger&)            = delete;
    event_trigger& operator=(const event_trigger&) = delete;
    event_trigger(event_trigger&&)                 = delete;
    event_trigger& operator=(event_trigger&&)      = delete;

    ~event_trigger() = default;

private:
    struct cb_t {
        event_trigger* self;
        void operator()(Args... args) const {
            if (self->action_) {
                self->action_(args...);
            }
        }
    };

    action_fn            action_;
    cb_t                 cb_{ this };
    signal_slot<Args...> slot_{};
};

} // namespace mpapp

#endif // MPAPP_TRIGGERS_EVENT_TRIGGER_HPP
