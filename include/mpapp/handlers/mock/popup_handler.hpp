// SPDX-License-Identifier: Apache-2.0
// Mock basic_popup handler. Records every property change and signal
// emission so tests can assert exact call sequences without any platform.

#ifndef MPAPP_HANDLERS_MOCK_POPUP_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_POPUP_HANDLER_HPP

#include <memory>
#include <optional>
#include <string>

#include "../../internal/basic_popup.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class popup_handler<platform::mock> : public mock_handler_base {
public:
    popup_handler()  = default;
    ~popup_handler() = default;

    popup_handler(const popup_handler&)            = delete;
    popup_handler& operator=(const popup_handler&) = delete;
    popup_handler(popup_handler&&)                 = delete;
    popup_handler& operator=(popup_handler&&)      = delete;

    // ----- Property mappers -----------------------------------------------

    void map_content(basic_popup& p) {
        record("content", p.content.get() ? std::string("set") : std::string("null"));
        p.content.changed.subscribe(content_slot_, content_cb_);
    }

    void map_is_open(basic_popup& p) {
        record_change("is_open", p.is_open.get());
        p.is_open.changed.subscribe(is_open_slot_, is_open_cb_);
    }

    void map_can_be_dismissed_by_tapping_outside(basic_popup& p) {
        record_change("can_be_dismissed_by_tapping_outside",
                      p.can_be_dismissed_by_tapping_outside.get());
        p.can_be_dismissed_by_tapping_outside.changed.subscribe(
            dismiss_slot_, dismiss_cb_);
    }

    // ----- Signal mappers ------------------------------------------------

    void map_opened(basic_popup& p) {
        p.opened.subscribe(opened_slot_, opened_cb_);
    }

    void map_closed(basic_popup& p) {
        p.closed.subscribe(closed_slot_, closed_cb_);
    }

    // ----- Gesture stub (RFC-0003) ----------------------------------------
    void map_gestures(basic_popup& /*p*/) noexcept {}

private:
    // --- content callback ---
    struct content_cb_t {
        popup_handler<platform::mock>* self;
        void operator()(const std::shared_ptr<view>& v) const {
            self->record("content", v ? std::string("set") : std::string("null"));
        }
    };

    content_cb_t                              content_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};

    // --- is_open callback ---
    mock_property_recorder<popup_handler<platform::mock>, bool> is_open_cb_{
        this, "is_open"};
    signal_slot<const bool&> is_open_slot_{};

    // --- can_be_dismissed_by_tapping_outside callback ---
    mock_property_recorder<popup_handler<platform::mock>, bool> dismiss_cb_{
        this, "can_be_dismissed_by_tapping_outside"};
    signal_slot<const bool&> dismiss_slot_{};

    // --- opened signal callback ---
    struct opened_cb_t {
        popup_handler<platform::mock>* self;
        void operator()() const { self->record_event("opened"); }
    };

    opened_cb_t   opened_cb_{this};
    signal_slot<> opened_slot_{};

    // --- closed signal callback ---
    struct closed_cb_t {
        popup_handler<platform::mock>* self;
        void operator()(const std::optional<std::string>& result) const {
            self->record("closed",
                         result.has_value() ? result.value() : std::string("<nullopt>"));
        }
    };

    closed_cb_t                                        closed_cb_{this};
    signal_slot<const std::optional<std::string>&>     closed_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_POPUP_HANDLER_HPP
