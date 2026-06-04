// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/CameraView.md
//
// Mock-handler tests for `mpapp::internal::basic_camera_view` (ADR-0008).
// Drives every property, command, and signal path and asserts the
// mock handler records exactly what was expected.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/internal/basic_camera_view.hpp>
#include <mpapp/handlers/mock/camera_view_handler.hpp>

namespace {

using camera_mock = mpapp::internal::camera_view_handler<mpapp::platform::mock>;

// Minimal stored callback for signal subscription (avoids binding a
// temporary lambda to signal::subscribe which requires an lvalue F).
struct string_sink {
    std::string* out = nullptr;
    void operator()(const std::string& s) const { *out = s; }
};

struct string_appender {
    std::vector<std::string>* out = nullptr;
    void operator()(const std::string& s) const { out->push_back(s); }
};

struct counter {
    int* n = nullptr;
    void operator()() const { ++*n; }
};

} // namespace

// ---------------------------------------------------------------------------
// flash
// ---------------------------------------------------------------------------

TEST_CASE("camera_view mock handler records initial flash on map",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_flash(cv);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"flash=off"});
}

TEST_CASE("camera_view mock handler records flash changes",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_flash(cv);
    h.clear_calls();

    cv.flash = mpapp::camera_flash::on;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"flash=on"});

    cv.flash = mpapp::camera_flash::on;    // idempotent
    REQUIRE(h.calls().size() == 1);

    cv.flash = mpapp::camera_flash::auto_;
    REQUIRE(h.calls_as_strings() ==
            (std::vector<std::string>{"flash=on", "flash=auto"}));
}

TEST_CASE("camera_view flash reverts to off from auto",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    cv.flash = mpapp::camera_flash::auto_;
    h.map_flash(cv);
    h.clear_calls();

    cv.flash = mpapp::camera_flash::off;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"flash=off"});
}

// ---------------------------------------------------------------------------
// position
// ---------------------------------------------------------------------------

TEST_CASE("camera_view mock handler records initial position on map",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_position(cv);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"position=rear"});
}

TEST_CASE("camera_view mock handler records position change to front",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_position(cv);
    h.clear_calls();

    cv.position = mpapp::camera_position::front;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"position=front"});

    cv.position = mpapp::camera_position::front;  // idempotent
    REQUIRE(h.calls().size() == 1);
}

// ---------------------------------------------------------------------------
// is_available
// ---------------------------------------------------------------------------

TEST_CASE("camera_view mock handler records initial is_available on map",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_is_available(cv);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_available=false"});
}

TEST_CASE("camera_view mock handler records is_available changes",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_is_available(cv);
    h.clear_calls();

    cv.is_available = true;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_available=true"});

    cv.is_available = true;  // idempotent
    REQUIRE(h.calls().size() == 1);

    cv.is_available = false;
    REQUIRE(h.calls_as_strings() ==
            (std::vector<std::string>{"is_available=true", "is_available=false"}));
}

// ---------------------------------------------------------------------------
// is_torch_on
// ---------------------------------------------------------------------------

TEST_CASE("camera_view mock handler records initial is_torch_on on map",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_is_torch_on(cv);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_torch_on=false"});
}

TEST_CASE("camera_view mock handler records torch toggle",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_is_torch_on(cv);
    h.clear_calls();

    cv.is_torch_on = true;
    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"is_torch_on=true"});

    cv.is_torch_on = false;
    REQUIRE(h.calls_as_strings() ==
            (std::vector<std::string>{"is_torch_on=true", "is_torch_on=false"}));
}

// ---------------------------------------------------------------------------
// zoom_factor
// ---------------------------------------------------------------------------

TEST_CASE("camera_view mock handler records initial zoom_factor on map",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_zoom_factor(cv);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"zoom_factor=1"});
}

TEST_CASE("camera_view mock handler records zoom_factor changes",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_zoom_factor(cv);
    h.clear_calls();

    cv.zoom_factor = 2.0;
    REQUIRE(h.calls().size() == 1);
    CHECK(h.calls()[0].property_name == "zoom_factor");
    CHECK(h.calls()[0].value_repr    == "2");

    cv.zoom_factor = 2.0;  // idempotent
    REQUIRE(h.calls().size() == 1);

    cv.zoom_factor = 1.5;
    REQUIRE(h.calls().size() == 2);
    CHECK(h.calls()[1].value_repr == "1.5");
}

// ---------------------------------------------------------------------------
// capture() -- success path
// ---------------------------------------------------------------------------

TEST_CASE("camera_view capture emits media_captured with default path",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;

    cv.is_available = true;

    std::string received_path;
    string_sink sink{&received_path};
    mpapp::signal_slot<std::string> slot;
    cv.media_captured.subscribe(slot, sink);

    cv.capture();

    CHECK(received_path == "captured.jpg");
}

TEST_CASE("camera_view capture emits media_captured with custom path",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;

    cv.is_available = true;
    cv.set_next_capture_path("photo_001.jpg");

    std::string received_path;
    string_sink sink{&received_path};
    mpapp::signal_slot<std::string> slot;
    cv.media_captured.subscribe(slot, sink);

    cv.capture();

    CHECK(received_path == "photo_001.jpg");
}

TEST_CASE("camera_view capture records via map_media_captured",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    cv.is_available = true;
    h.map_media_captured(cv);

    cv.capture();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"media_captured"});
}

TEST_CASE("camera_view multiple captures accumulate in call log",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    cv.is_available = true;
    h.map_media_captured(cv);

    cv.capture();
    cv.capture();

    REQUIRE(h.calls_as_strings() ==
            (std::vector<std::string>{"media_captured", "media_captured"}));
}

// ---------------------------------------------------------------------------
// capture() -- error path (camera not available)
// ---------------------------------------------------------------------------

TEST_CASE("camera_view capture emits camera_error when not available",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    // is_available defaults to false.

    std::string error_msg;
    string_sink sink{&error_msg};
    mpapp::signal_slot<std::string> slot;
    cv.camera_error.subscribe(slot, sink);

    cv.capture();

    CHECK(error_msg == "camera not available");
}

TEST_CASE("camera_view capture error records via map_camera_error",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    // is_available is false.
    h.map_camera_error(cv);

    cv.capture();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"camera_error"});
}

TEST_CASE("camera_view capture does not emit media_captured when unavailable",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_media_captured(cv);
    h.map_camera_error(cv);

    cv.capture();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"camera_error"});
}

// ---------------------------------------------------------------------------
// start_preview / stop_preview
// ---------------------------------------------------------------------------

TEST_CASE("camera_view start_preview emits preview_started signal",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_preview_started(cv);
    cv.start_preview();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"start_preview"});
}

TEST_CASE("camera_view stop_preview emits preview_stopped signal",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_preview_stopped(cv);
    cv.stop_preview();

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{"stop_preview"});
}

TEST_CASE("camera_view start then stop preview records sequence",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_preview_started(cv);
    h.map_preview_stopped(cv);
    cv.start_preview();
    cv.stop_preview();

    REQUIRE(h.calls_as_strings() ==
            (std::vector<std::string>{"start_preview", "stop_preview"}));
}

// ---------------------------------------------------------------------------
// Combined property + command sequence
// ---------------------------------------------------------------------------

TEST_CASE("camera_view combined property changes and capture sequence",
          "[mock][camera_view][sequence]") {
    mpapp::internal::basic_camera_view cv;
    camera_mock h;

    h.map_flash(cv);
    h.map_position(cv);
    h.map_is_available(cv);
    h.map_zoom_factor(cv);
    h.map_media_captured(cv);
    h.clear_calls();

    cv.flash        = mpapp::camera_flash::auto_;
    cv.position     = mpapp::camera_position::front;
    cv.is_available = true;
    cv.zoom_factor  = 3.0;

    cv.capture();

    REQUIRE(h.calls().size() == 5);
    CHECK(h.calls()[0].property_name == "flash");
    CHECK(h.calls()[1].property_name == "position");
    CHECK(h.calls()[2].property_name == "is_available");
    CHECK(h.calls()[3].property_name == "zoom_factor");
    CHECK(h.calls()[4].property_name == "media_captured");
    CHECK(h.calls()[4].has_value == false);
}

// ---------------------------------------------------------------------------
// enum to_string helpers
// ---------------------------------------------------------------------------

TEST_CASE("camera_flash to_string covers all values",
          "[mock][camera_view][enum]") {
    CHECK(mpapp::to_string(mpapp::camera_flash::off)   == "off");
    CHECK(mpapp::to_string(mpapp::camera_flash::on)    == "on");
    CHECK(mpapp::to_string(mpapp::camera_flash::auto_) == "auto");
}

TEST_CASE("camera_position to_string covers all values",
          "[mock][camera_view][enum]") {
    CHECK(mpapp::to_string(mpapp::camera_position::front) == "front");
    CHECK(mpapp::to_string(mpapp::camera_position::rear)  == "rear");
}

// ---------------------------------------------------------------------------
// has_handler / set_handler
// ---------------------------------------------------------------------------

TEST_CASE("camera_view has_handler reflects set_handler",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;
    // has_handler is false before any set_handler; test uses the mock
    // handler only through the mapper API, not through set_handler, so
    // this just verifies the state flag.
    CHECK(!cv.has_handler());
}

// ---------------------------------------------------------------------------
// next_capture_path helper
// ---------------------------------------------------------------------------

TEST_CASE("camera_view next_capture_path default and setter",
          "[mock][camera_view]") {
    mpapp::internal::basic_camera_view cv;

    CHECK(cv.next_capture_path() == "captured.jpg");

    cv.set_next_capture_path("video_042.mp4");
    CHECK(cv.next_capture_path() == "video_042.mp4");
}
