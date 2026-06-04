// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MediaElement.md
//
// `mpapp::internal::basic_media_element` — cross-platform media-playback
// surface. Mirrors the CommunityToolkit.Maui MediaElement API:
// source, playback state, position/duration, volume, speed, loop, autoplay
// and show_controls observables, plus play/pause/stop/seek commands and
// state_changed/media_opened/media_ended/position_changed signals.
// Mock surface (P2 / ADR-0008).

#ifndef MPAPP_INTERNAL_BASIC_MEDIA_ELEMENT_HPP
#define MPAPP_INTERNAL_BASIC_MEDIA_ELEMENT_HPP

#include <cstdint>
#include <string>
#include <string_view>

#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_MEDIA_ELEMENT_HAS_STD_FORMAT 1
#endif

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

// Mirrors CommunityToolkit.Maui MediaElementState.
enum class media_state : std::uint8_t {
    none       = 0,
    opening    = 1,
    buffering  = 2,
    playing    = 3,
    paused     = 4,
    stopped    = 5,
    failed     = 6,
};

constexpr std::string_view to_string(media_state s) noexcept {
    switch (s) {
        case media_state::none:      return "none";
        case media_state::opening:   return "opening";
        case media_state::buffering: return "buffering";
        case media_state::playing:   return "playing";
        case media_state::paused:    return "paused";
        case media_state::stopped:   return "stopped";
        case media_state::failed:    return "failed";
    }
    return "?";
}

} // namespace mpapp

namespace mpapp::internal {

// Forward-declared so basic_media_element can name it as a pointer member
// without a circular include. Specialisations live in
// `mpapp/handlers/<platform>/media_element_handler.hpp` and
// `mpapp/handlers/mock/media_element_handler.hpp`.
template <class Platform = platform::current>
class media_element_handler;

class basic_media_element : public view {
public:
    basic_media_element() = default;

    basic_media_element(const basic_media_element&)            = delete;
    basic_media_element& operator=(const basic_media_element&) = delete;
    basic_media_element(basic_media_element&&)                 = delete;
    basic_media_element& operator=(basic_media_element&&)      = delete;

    // ----- Properties -------------------------------------------------------

    // Media source URI or file path.
    Observable<std::string>    source{""};

    // Current playback state. Primarily driven by the platform handler;
    // tests may set it directly to simulate state transitions.
    Observable<media_state>    state{media_state::none};

    // Playback position in seconds.
    Observable<double>         position_seconds{0.0};

    // Total duration in seconds. Set by the handler once the media is opened.
    Observable<double>         duration_seconds{0.0};

    // Audio volume: 0.0 (silent) .. 1.0 (full).
    Observable<double>         volume{1.0};

    // Playback speed multiplier: 1.0 = normal speed.
    Observable<double>         speed{1.0};

    // Whether the media loops back to the start after it ends.
    Observable<bool>           is_looping{false};

    // Whether playback begins automatically when source is set.
    Observable<bool>           should_autoplay{false};

    // Whether the native media controls are visible.
    Observable<bool>           show_controls{true};

    // ----- Command signals --------------------------------------------------
    // These signals are emitted by play/pause/stop/seek so the platform
    // handler can subscribe and route to the native player. The mock handler
    // records each emission as a call_record (event or value-bearing).

    mpapp::signal<>            play_requested;
    mpapp::signal<>            pause_requested;
    mpapp::signal<>            stop_requested;
    mpapp::signal<double>      seek_requested;   // carries target position in seconds

    // Convenience methods: emit the corresponding command signal.
    void play()  { play_requested.emit(); }
    void pause() { pause_requested.emit(); }
    void stop()  { stop_requested.emit(); }
    void seek(double position_seconds_target) {
        seek_requested.emit(position_seconds_target);
    }

    // ----- Output signals ---------------------------------------------------

    // Emitted when the playback state transitions.
    mpapp::signal<media_state>  state_changed;

    // Emitted once the media source has been opened and metadata is available.
    mpapp::signal<>             media_opened;

    // Emitted when playback reaches the end of the media.
    mpapp::signal<>             media_ended;

    // Emitted periodically as playback position advances.
    mpapp::signal<double>       position_changed;

    // ----- Handler attachment -----------------------------------------------
    [[nodiscard]] media_element_handler<platform::current>&
    handler() noexcept { return *handler_; }

    [[nodiscard]] const media_element_handler<platform::current>&
    handler() const noexcept { return *handler_; }

    [[nodiscard]] bool has_handler() const noexcept { return handler_ != nullptr; }

    void set_handler(media_element_handler<platform::current>& h) noexcept {
        handler_ = &h;
    }

private:
    media_element_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#ifdef MPAPP_MEDIA_ELEMENT_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::media_state> : std::formatter<std::string_view> {
    auto format(mpapp::media_state s, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(s), ctx);
    }
};

#endif // MPAPP_MEDIA_ELEMENT_HAS_STD_FORMAT

#endif // MPAPP_INTERNAL_BASIC_MEDIA_ELEMENT_HPP
