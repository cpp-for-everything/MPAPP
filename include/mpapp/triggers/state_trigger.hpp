// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0008-triggers.md
//
// `mpapp::state_trigger` — drives a `visual_state_manager` from a
// boolean condition. Counterpart to MAUI's StateTrigger (the bridge
// between the trigger family and the Visual State Manager, RFC-0006):
// when the condition is true the target transitions to `active_state`,
// when false to `inactive_state` (skipped if empty). Re-evaluates on
// every change of the condition Observable. Owns its slot; no macros;
// platform-neutral.

#ifndef MPAPP_TRIGGERS_STATE_TRIGGER_HPP
#define MPAPP_TRIGGERS_STATE_TRIGGER_HPP

#include <string>
#include <string_view>
#include <utility>

#include "../observable.hpp"
#include "../resources/visual_state_manager.hpp"
#include "../signal.hpp"

namespace mpapp {

class view;

class state_trigger {
public:
    state_trigger(Observable<bool>& condition, view& target,
                  visual_state_manager& vsm,
                  std::string active_state, std::string inactive_state = {})
        : condition_{ &condition }
        , target_{ &target }
        , vsm_{ &vsm }
        , active_state_{ std::move(active_state) }
        , inactive_state_{ std::move(inactive_state) } {
        evaluate();
        condition_->changed.subscribe(slot_, cb_);
    }

    state_trigger(const state_trigger&)            = delete;
    state_trigger& operator=(const state_trigger&) = delete;
    state_trigger(state_trigger&&)                 = delete;
    state_trigger& operator=(state_trigger&&)      = delete;

    ~state_trigger() = default;

private:
    void evaluate() {
        const std::string& s = condition_->get() ? active_state_ : inactive_state_;
        if (!s.empty()) {
            vsm_->go_to_state(*target_, std::string_view{ s });
        }
    }

    struct cb_t {
        state_trigger* self;
        void operator()(bool) const { self->evaluate(); }
    };

    Observable<bool>*       condition_;
    view*                   target_;
    visual_state_manager*   vsm_;
    std::string             active_state_;
    std::string             inactive_state_;
    cb_t                    cb_{ this };
    signal_slot<const bool&> slot_{};
};

} // namespace mpapp

#endif // MPAPP_TRIGGERS_STATE_TRIGGER_HPP
