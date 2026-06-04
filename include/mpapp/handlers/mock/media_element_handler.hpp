// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MediaElement.md
//
// `media_element_handler<platform::mock>` — records property mappers and
// playback-command invocations for `basic_media_element` so unit tests can
// assert the exact sequence of calls the framework would route to a real
// native handler.

#ifndef MPAPP_HANDLERS_MOCK_MEDIA_ELEMENT_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_MEDIA_ELEMENT_HANDLER_HPP

#include "../../internal/basic_media_element.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class media_element_handler<platform::mock>
    : public mock_handler_base {
public:
    media_element_handler()  = default;
    ~media_element_handler() = default;

    media_element_handler(const media_element_handler&)            = delete;
    media_element_handler& operator=(const media_element_handler&) = delete;
    media_element_handler(media_element_handler&&)                 = delete;
    media_element_handler& operator=(media_element_handler&&)      = delete;

    // ----- Property mappers -------------------------------------------------
    // Each map_* subscribes the corresponding Observable, records the initial
    // value immediately, then records every subsequent change automatically.

    void map_source(basic_media_element& m) {
        record_change("source", m.source.get());
        m.source.changed.subscribe(source_slot_, source_cb_);
    }

    void map_state(basic_media_element& m) {
        record_change("state", m.state.get());
        m.state.changed.subscribe(state_slot_, state_cb_);
    }

    void map_position_seconds(basic_media_element& m) {
        record_change("position_seconds", m.position_seconds.get());
        m.position_seconds.changed.subscribe(position_seconds_slot_, position_seconds_cb_);
    }

    void map_duration_seconds(basic_media_element& m) {
        record_change("duration_seconds", m.duration_seconds.get());
        m.duration_seconds.changed.subscribe(duration_seconds_slot_, duration_seconds_cb_);
    }

    void map_volume(basic_media_element& m) {
        record_change("volume", m.volume.get());
        m.volume.changed.subscribe(volume_slot_, volume_cb_);
    }

    void map_speed(basic_media_element& m) {
        record_change("speed", m.speed.get());
        m.speed.changed.subscribe(speed_slot_, speed_cb_);
    }

    void map_is_looping(basic_media_element& m) {
        record_change("is_looping", m.is_looping.get());
        m.is_looping.changed.subscribe(is_looping_slot_, is_looping_cb_);
    }

    void map_should_autoplay(basic_media_element& m) {
        record_change("should_autoplay", m.should_autoplay.get());
        m.should_autoplay.changed.subscribe(should_autoplay_slot_, should_autoplay_cb_);
    }

    void map_show_controls(basic_media_element& m) {
        record_change("show_controls", m.show_controls.get());
        m.show_controls.changed.subscribe(show_controls_slot_, show_controls_cb_);
    }

    // ----- Command mappers --------------------------------------------------
    // Subscribes to the surface's command signals so that calls to
    // m.play() / m.pause() / m.stop() / m.seek(...) are recorded.

    void map_play(basic_media_element& m) {
        m.play_requested.subscribe(play_slot_, play_cb_);
    }

    void map_pause(basic_media_element& m) {
        m.pause_requested.subscribe(pause_slot_, pause_cb_);
    }

    void map_stop(basic_media_element& m) {
        m.stop_requested.subscribe(stop_slot_, stop_cb_);
    }

    void map_seek(basic_media_element& m) {
        m.seek_requested.subscribe(seek_slot_, seek_cb_);
    }

    // RFC-0003 stub: per-platform real gesture wire-up is
    // pending the platform's real-handler task. No-op today
    // so the wrapper ctor's unconditional
    // `embedded_handler_.map_gestures(*this);` links.
    void map_gestures(basic_media_element& /*m*/) noexcept {}

private:
    using self_t = media_element_handler<platform::mock>;

    // --- Property change recorders ---
    mock_property_recorder<self_t, std::string> source_cb_{this, "source"};
    signal_slot<const std::string&>             source_slot_{};

    mock_property_recorder<self_t, media_state> state_cb_{this, "state"};
    signal_slot<const media_state&>             state_slot_{};

    mock_property_recorder<self_t, double>      position_seconds_cb_{this, "position_seconds"};
    signal_slot<const double&>                  position_seconds_slot_{};

    mock_property_recorder<self_t, double>      duration_seconds_cb_{this, "duration_seconds"};
    signal_slot<const double&>                  duration_seconds_slot_{};

    mock_property_recorder<self_t, double>      volume_cb_{this, "volume"};
    signal_slot<const double&>                  volume_slot_{};

    mock_property_recorder<self_t, double>      speed_cb_{this, "speed"};
    signal_slot<const double&>                  speed_slot_{};

    mock_property_recorder<self_t, bool>        is_looping_cb_{this, "is_looping"};
    signal_slot<const bool&>                    is_looping_slot_{};

    mock_property_recorder<self_t, bool>        should_autoplay_cb_{this, "should_autoplay"};
    signal_slot<const bool&>                    should_autoplay_slot_{};

    mock_property_recorder<self_t, bool>        show_controls_cb_{this, "show_controls"};
    signal_slot<const bool&>                    show_controls_slot_{};

    // --- Command recorders ---
    struct play_recorder {
        self_t* self;
        void operator()() const { self->record_event("play"); }
    };
    struct pause_recorder {
        self_t* self;
        void operator()() const { self->record_event("pause"); }
    };
    struct stop_recorder {
        self_t* self;
        void operator()() const { self->record_event("stop"); }
    };
    struct seek_recorder {
        self_t* self;
        void operator()(double pos) const { self->record_change("seek", pos); }
    };

    play_recorder  play_cb_{this};
    signal_slot<>  play_slot_{};

    pause_recorder pause_cb_{this};
    signal_slot<>  pause_slot_{};

    stop_recorder  stop_cb_{this};
    signal_slot<>  stop_slot_{};

    seek_recorder  seek_cb_{this};
    signal_slot<double> seek_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_HANDLERS_MOCK_MEDIA_ELEMENT_HANDLER_HPP
