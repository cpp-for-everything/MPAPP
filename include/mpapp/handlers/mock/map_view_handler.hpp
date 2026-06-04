// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MapView.md
//
// `map_view_handler<platform::mock>` — records property mappers and
// signal subscriptions for `basic_map_view` (map_kind, geo_point center,
// zoom, feature toggles, pin_clicked, map_clicked).

#ifndef MPAPP_HANDLERS_MOCK_MAP_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MAP_VIEW_HANDLER_HPP

#include "../../internal/basic_map_view.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class map_view_handler<platform::mock>
    : public mock_handler_base {
public:
    map_view_handler()  = default;
    ~map_view_handler() = default;

    map_view_handler(const map_view_handler&)            = delete;
    map_view_handler& operator=(const map_view_handler&) = delete;
    map_view_handler(map_view_handler&&)                 = delete;
    map_view_handler& operator=(map_view_handler&&)      = delete;

    // ----- Property mappers ------------------------------------------------

    void map_kind(basic_map_view& m) {
        record_change("kind", m.kind.get());
        m.kind.changed.subscribe(kind_slot_, kind_cb_);
    }

    void map_center(basic_map_view& m) {
        record_change("center", m.center.get());
        m.center.changed.subscribe(center_slot_, center_cb_);
    }

    void map_zoom(basic_map_view& m) {
        record_change("zoom", m.zoom.get());
        m.zoom.changed.subscribe(zoom_slot_, zoom_cb_);
    }

    void map_is_showing_user(basic_map_view& m) {
        record_change("is_showing_user", m.is_showing_user.get());
        m.is_showing_user.changed.subscribe(showing_user_slot_, showing_user_cb_);
    }

    void map_is_traffic_enabled(basic_map_view& m) {
        record_change("is_traffic_enabled", m.is_traffic_enabled.get());
        m.is_traffic_enabled.changed.subscribe(traffic_slot_, traffic_cb_);
    }

    void map_is_scroll_enabled(basic_map_view& m) {
        record_change("is_scroll_enabled", m.is_scroll_enabled.get());
        m.is_scroll_enabled.changed.subscribe(scroll_slot_, scroll_cb_);
    }

    void map_is_zoom_enabled(basic_map_view& m) {
        record_change("is_zoom_enabled", m.is_zoom_enabled.get());
        m.is_zoom_enabled.changed.subscribe(zoom_enabled_slot_, zoom_enabled_cb_);
    }

    // ----- Signal mappers --------------------------------------------------

    // Subscribe to pin_clicked so tests can observe the signal.
    void map_pin_clicked(basic_map_view& m) {
        m.pin_clicked.subscribe(pin_clicked_slot_, pin_clicked_cb_);
    }

    // Subscribe to map_clicked so tests can observe the signal.
    void map_map_clicked(basic_map_view& m) {
        m.map_clicked.subscribe(map_clicked_slot_, map_clicked_cb_);
    }

    // RFC-0003 stub: per-platform real gesture wire-up pending.
    void map_gestures(basic_map_view& /*m*/) noexcept {}

private:
    // Recorders for Observable properties.
    mock_property_recorder<map_view_handler<platform::mock>, mpapp::map_kind> kind_cb_{
        this, "kind"};
    signal_slot<const mpapp::map_kind&> kind_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, mpapp::geo_point> center_cb_{
        this, "center"};
    signal_slot<const mpapp::geo_point&> center_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, double> zoom_cb_{
        this, "zoom"};
    signal_slot<const double&> zoom_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, bool> showing_user_cb_{
        this, "is_showing_user"};
    signal_slot<const bool&> showing_user_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, bool> traffic_cb_{
        this, "is_traffic_enabled"};
    signal_slot<const bool&> traffic_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, bool> scroll_cb_{
        this, "is_scroll_enabled"};
    signal_slot<const bool&> scroll_slot_{};

    mock_property_recorder<map_view_handler<platform::mock>, bool> zoom_enabled_cb_{
        this, "is_zoom_enabled"};
    signal_slot<const bool&> zoom_enabled_slot_{};

    // Recorders for signals (pin_clicked, map_clicked).
    struct pin_clicked_recorder {
        map_view_handler<platform::mock>* self = nullptr;
        void operator()(const map_pin& p) const {
            self->record_change("pin_clicked", p);
        }
    };

    struct map_clicked_recorder {
        map_view_handler<platform::mock>* self = nullptr;
        void operator()(const geo_point& gp) const {
            self->record_change("map_clicked", gp);
        }
    };

    pin_clicked_recorder      pin_clicked_cb_{this};
    signal_slot<const map_pin&> pin_clicked_slot_{};

    map_clicked_recorder        map_clicked_cb_{this};
    signal_slot<const geo_point&> map_clicked_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_MAP_VIEW_HANDLER_HPP
