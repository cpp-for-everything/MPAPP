// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0006-visual-state-manager.md
//
// `mpapp::visual_state_input_router` — the "VSM input-routing" layer the
// visual_state_manager header calls out as a follow-up: it maps system
// input state (enabled / pressed / pointer-over / focused) onto the
// canonical `visual_states` names and drives `vsm.go_to_state(view, …)`
// automatically, so controls light up their Pressed / PointerOver /
// Disabled / Focused states without the app calling go_to_state by hand
// (mirrors MAUI's VisualStateManager auto-routing of CommonStates).
//
// This is pure, platform-neutral routing logic (like the VSM itself —
// no native widget, no handler). It:
//   * subscribes the view's `is_enabled` Observable and routes it to
//     Disabled / Normal automatically (works on every platform with zero
//     native wiring), and
//   * exposes `set_pressed` / `set_pointer_over` / `set_focused` for the
//     platform's native pointer/focus events (or a pointer gesture
//     recognizer) to call. The priority order matches MAUI's
//     CommonStates: Disabled > Pressed > PointerOver > Focused > Normal.
//
// Tests drive the setters directly and assert the VSM transitioned; on a
// real platform the pointer/focus events feed the same setters.

#ifndef MPAPP_RESOURCES_VISUAL_STATE_INPUT_ROUTER_HPP
#define MPAPP_RESOURCES_VISUAL_STATE_INPUT_ROUTER_HPP

#include <string_view>

#include "../signal.hpp"
#include "../view.hpp"
#include "visual_state_manager.hpp"

namespace mpapp {

class visual_state_input_router {
public:
    // Binds the router to a view + its VSM. Routes the view's current
    // `is_enabled` immediately, then on every change.
    visual_state_input_router(view& v, visual_state_manager& vsm)
        : view_(&v), vsm_(&vsm) {
        enabled_ = v.is_enabled.get();
        v.is_enabled.changed.subscribe(enabled_slot_, enabled_cb_);
        refresh();
    }

    visual_state_input_router(const visual_state_input_router&)            = delete;
    visual_state_input_router& operator=(const visual_state_input_router&) = delete;
    visual_state_input_router(visual_state_input_router&&)                 = delete;
    visual_state_input_router& operator=(visual_state_input_router&&)      = delete;

    // ----- Input sources (native pointer/focus events call these) -------
    void set_pressed(bool v)      { if (pressed_ == v) return; pressed_ = v; refresh(); }
    void set_pointer_over(bool v) { if (over_   == v) return; over_   = v; refresh(); }
    void set_focused(bool v)      { if (focused_ == v) return; focused_ = v; refresh(); }

    // The canonical state the router would apply right now.
    [[nodiscard]] std::string_view current() const noexcept { return resolve_(); }

private:
    [[nodiscard]] std::string_view resolve_() const noexcept {
        if (!enabled_) return visual_states::disabled;
        if (pressed_)  return visual_states::pressed;
        if (over_)     return visual_states::pointer_over;
        if (focused_)  return visual_states::focused;
        return visual_states::normal;
    }

    void refresh() {
        if (view_ != nullptr && vsm_ != nullptr) {
            vsm_->go_to_state(*view_, resolve_());
        }
    }

    void on_enabled(bool e) { enabled_ = e; refresh(); }

    struct enabled_cb_t {
        visual_state_input_router* self;
        void operator()(bool e) const { self->on_enabled(e); }
    };

    view*                    view_    = nullptr;
    visual_state_manager*    vsm_     = nullptr;
    bool                     enabled_ = true;
    bool                     pressed_ = false;
    bool                     over_    = false;
    bool                     focused_ = false;
    enabled_cb_t             enabled_cb_{this};
    signal_slot<const bool&> enabled_slot_{};
};

} // namespace mpapp

#endif // MPAPP_RESOURCES_VISUAL_STATE_INPUT_ROUTER_HPP
