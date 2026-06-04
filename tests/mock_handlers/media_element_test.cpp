// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/MediaElement.md
//
// Mock-handler tests for `mpapp::internal::basic_media_element` (ADR-0008).
// Covers every property mapper, every command mapper, and every signal.
// AAA structure throughout.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_media_element.hpp>
#include <mpapp/handlers/mock/media_element_handler.hpp>

using namespace mpapp;
using media_mock = internal::media_element_handler<platform::mock>;

// ---------------------------------------------------------------------------
// map_* initial-value recording
// ---------------------------------------------------------------------------

TEST_CASE("media_element mock handler records initial source on map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    m.source = "https://example.com/clip.mp4";

    // Act
    h.map_source(m);

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "source");
    CHECK(h.calls()[0].value_repr    == "https://example.com/clip.mp4");
}

TEST_CASE("media_element mock handler records initial state on map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;

    // Act
    h.map_state(m);

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "state");
    CHECK(h.calls()[0].value_repr    == "none");
}

TEST_CASE("media_element mock handler records initial numeric properties",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;

    // Act
    h.map_position_seconds(m);
    h.map_duration_seconds(m);
    h.map_volume(m);
    h.map_speed(m);

    // Assert
    REQUIRE(h.calls().size() == 4);
    CHECK(h.calls()[0].property_name == "position_seconds");
    CHECK(h.calls()[0].value_repr    == "0");
    CHECK(h.calls()[1].property_name == "duration_seconds");
    CHECK(h.calls()[1].value_repr    == "0");
    CHECK(h.calls()[2].property_name == "volume");
    CHECK(h.calls()[2].value_repr    == "1");
    CHECK(h.calls()[3].property_name == "speed");
    CHECK(h.calls()[3].value_repr    == "1");
}

TEST_CASE("media_element mock handler records initial bool properties",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;

    // Act
    h.map_is_looping(m);
    h.map_should_autoplay(m);
    h.map_show_controls(m);

    // Assert
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].property_name == "is_looping");
    CHECK(h.calls()[0].value_repr    == "false");
    CHECK(h.calls()[1].property_name == "should_autoplay");
    CHECK(h.calls()[1].value_repr    == "false");
    CHECK(h.calls()[2].property_name == "show_controls");
    CHECK(h.calls()[2].value_repr    == "true");
}

// ---------------------------------------------------------------------------
// Observable change tracking
// ---------------------------------------------------------------------------

TEST_CASE("media_element source changes are recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_source(m);
    h.clear_calls();

    // Act
    m.source = "file:///media/test.mp4";
    m.source = "file:///media/test.mp4";   // idempotent -- no extra record
    m.source = "file:///media/other.mp4";

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "file:///media/test.mp4");
    CHECK(h.calls()[1].value_repr == "file:///media/other.mp4");
}

TEST_CASE("media_element state changes are recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_state(m);
    h.clear_calls();

    // Act
    m.state = media_state::opening;
    m.state = media_state::buffering;
    m.state = media_state::playing;
    m.state = media_state::playing;   // idempotent -- no extra record

    // Assert
    REQUIRE(h.calls().size() == 3);
    CHECK(h.calls()[0].value_repr == "opening");
    CHECK(h.calls()[1].value_repr == "buffering");
    CHECK(h.calls()[2].value_repr == "playing");
}

TEST_CASE("media_element volume changes are recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_volume(m);
    h.clear_calls();

    // Act
    m.volume = 0.5;
    m.volume = 0.5;   // idempotent
    m.volume = 0.0;

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "0.5");
    CHECK(h.calls()[1].value_repr == "0");
}

TEST_CASE("media_element speed changes are recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_speed(m);
    h.clear_calls();

    // Act
    m.speed = 2.0;
    m.speed = 0.5;

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "2");
    CHECK(h.calls()[1].value_repr == "0.5");
}

TEST_CASE("media_element position_seconds changes are recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_position_seconds(m);
    h.clear_calls();

    // Act
    m.position_seconds = 10.5;
    m.position_seconds = 20.0;

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "10.5");
    CHECK(h.calls()[1].value_repr == "20");
}

TEST_CASE("media_element duration_seconds change is recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_duration_seconds(m);
    h.clear_calls();

    // Act
    m.duration_seconds = 120.0;

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "duration_seconds");
    CHECK(h.calls()[0].value_repr    == "120");
}

TEST_CASE("media_element is_looping toggle is recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_is_looping(m);
    h.clear_calls();

    // Act
    m.is_looping = true;
    m.is_looping = true;   // idempotent
    m.is_looping = false;

    // Assert
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[0].value_repr == "true");
    CHECK(h.calls()[1].value_repr == "false");
}

TEST_CASE("media_element should_autoplay change is recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_should_autoplay(m);
    h.clear_calls();

    // Act
    m.should_autoplay = true;

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "true");
}

TEST_CASE("media_element show_controls change is recorded after map",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_show_controls(m);
    h.clear_calls();

    // Act
    m.show_controls = false;

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].value_repr == "false");
}

// ---------------------------------------------------------------------------
// Playback commands: play / pause / stop / seek
// ---------------------------------------------------------------------------

TEST_CASE("media_element play command is recorded by mock handler",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_play(m);

    // Act
    m.play();

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "play");
    CHECK(h.calls()[0].has_value     == false);
}

TEST_CASE("media_element pause command is recorded by mock handler",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_pause(m);

    // Act
    m.pause();

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "pause");
    CHECK(h.calls()[0].has_value     == false);
}

TEST_CASE("media_element stop command is recorded by mock handler",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_stop(m);

    // Act
    m.stop();

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "stop");
    CHECK(h.calls()[0].has_value     == false);
}

TEST_CASE("media_element seek command records target position",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_seek(m);

    // Act
    m.seek(42.5);

    // Assert
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "seek");
    CHECK(h.calls()[0].value_repr    == "42.5");
    CHECK(h.calls()[0].has_value     == true);
}

TEST_CASE("media_element commands with no handler subscribed are silent",
          "[mock][media_element]") {
    // Arrange -- no handler mapped
    internal::basic_media_element m;

    // Act + Assert -- must not crash or throw
    m.play();
    m.pause();
    m.stop();
    m.seek(0.0);
    CHECK(true);  // reached here = no crash
}

// ---------------------------------------------------------------------------
// Command sequence
// ---------------------------------------------------------------------------

TEST_CASE("media_element play-pause-stop sequence is recorded in order",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_play(m);
    h.map_pause(m);
    h.map_stop(m);

    // Act
    m.play();
    m.pause();
    m.stop();

    // Assert
    auto s = h.calls_as_strings();
    REQUIRE(s.size() == 3);
    CHECK(s[0] == "play");
    CHECK(s[1] == "pause");
    CHECK(s[2] == "stop");
}

TEST_CASE("media_element seek records consecutive positions in order",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_seek(m);

    // Act
    m.seek(0.0);
    m.seek(30.0);
    m.seek(60.0);

    // Assert
    auto s = h.calls_as_strings();
    REQUIRE(s.size() == 3);
    CHECK(s[0] == "seek=0");
    CHECK(s[1] == "seek=30");
    CHECK(s[2] == "seek=60");
}

// ---------------------------------------------------------------------------
// Signals: state_changed / media_opened / media_ended / position_changed
// ---------------------------------------------------------------------------

TEST_CASE("media_element state_changed signal fires on explicit emit",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_state received = media_state::none;
    int fire_count = 0;

    struct cb_t {
        media_state* received;
        int*         fire_count;
        void operator()(media_state s) const { *received = s; ++(*fire_count); }
    } cb{&received, &fire_count};

    signal_slot<media_state> slot;
    m.state_changed.subscribe(slot, cb);

    // Act
    m.state_changed.emit(media_state::playing);

    // Assert
    CHECK(fire_count == 1);
    CHECK(received   == media_state::playing);
}

TEST_CASE("media_element media_opened signal fires exactly once",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    int opened_count = 0;

    struct cb_t {
        int* count;
        void operator()() const { ++(*count); }
    } cb{&opened_count};

    signal_slot<> slot;
    m.media_opened.subscribe(slot, cb);

    // Act
    m.media_opened.emit();

    // Assert
    CHECK(opened_count == 1);
}

TEST_CASE("media_element media_ended signal fires exactly once",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    int ended_count = 0;

    struct cb_t {
        int* count;
        void operator()() const { ++(*count); }
    } cb{&ended_count};

    signal_slot<> slot;
    m.media_ended.subscribe(slot, cb);

    // Act
    m.media_ended.emit();

    // Assert
    CHECK(ended_count == 1);
}

TEST_CASE("media_element position_changed signal delivers correct position",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    double received_pos = -1.0;

    struct cb_t {
        double* pos;
        void operator()(double p) const { *pos = p; }
    } cb{&received_pos};

    signal_slot<double> slot;
    m.position_changed.subscribe(slot, cb);

    // Act
    m.position_changed.emit(15.75);

    // Assert
    CHECK(received_pos == 15.75);
}

TEST_CASE("media_element position_changed fires multiple times for successive positions",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    std::vector<double> positions;

    struct cb_t {
        std::vector<double>* positions;
        void operator()(double p) const { positions->push_back(p); }
    } cb{&positions};

    signal_slot<double> slot;
    m.position_changed.subscribe(slot, cb);

    // Act
    m.position_changed.emit(1.0);
    m.position_changed.emit(2.0);
    m.position_changed.emit(3.0);

    // Assert
    REQUIRE(positions.size() == 3);
    CHECK(positions[0] == 1.0);
    CHECK(positions[1] == 2.0);
    CHECK(positions[2] == 3.0);
}

// ---------------------------------------------------------------------------
// to_string / media_state enum coverage
// ---------------------------------------------------------------------------

TEST_CASE("media_state to_string returns correct string for every value",
          "[mock][media_element]") {
    CHECK(to_string(media_state::none)      == "none");
    CHECK(to_string(media_state::opening)   == "opening");
    CHECK(to_string(media_state::buffering) == "buffering");
    CHECK(to_string(media_state::playing)   == "playing");
    CHECK(to_string(media_state::paused)    == "paused");
    CHECK(to_string(media_state::stopped)   == "stopped");
    CHECK(to_string(media_state::failed)    == "failed");
}

// ---------------------------------------------------------------------------
// has_handler guard
// ---------------------------------------------------------------------------

TEST_CASE("media_element has_handler reflects whether handler is set",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    internal::media_element_handler<platform::mock> h;

    // Assert: before set -- cannot call set_handler with mock on current platform,
    // but has_handler() must return false by default.
    CHECK_FALSE(m.has_handler());
}

// ---------------------------------------------------------------------------
// Mixed property + command sequence
// ---------------------------------------------------------------------------

TEST_CASE("media_element mixed source-set and play-seek sequence is recorded in order",
          "[mock][media_element]") {
    // Arrange
    internal::basic_media_element m;
    media_mock h;
    h.map_source(m);
    h.map_state(m);
    h.map_play(m);
    h.map_seek(m);
    h.map_pause(m);
    h.clear_calls();

    // Act
    m.source = "file:///video.mp4";
    m.state  = media_state::playing;
    m.play();
    m.seek(5.0);
    m.state  = media_state::paused;
    m.pause();

    // Assert
    auto s = h.calls_as_strings();
    REQUIRE(s.size() == 6);
    CHECK(s[0] == "source=file:///video.mp4");
    CHECK(s[1] == "state=playing");
    CHECK(s[2] == "play");
    CHECK(s[3] == "seek=5");
    CHECK(s[4] == "state=paused");
    CHECK(s[5] == "pause");
}
