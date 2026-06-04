// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::media_picker (RFC-0013 Essentials).

#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/media_picker.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static media_file make_photo(const std::string& name = "photo.jpg") {
    return media_file{
        "/storage/pictures/" + name,
        name,
        "image/jpeg"
    };
}

static media_file make_video(const std::string& name = "video.mp4") {
    return media_file{
        "/storage/videos/" + name,
        name,
        "video/mp4"
    };
}

// ---------------------------------------------------------------------------
// media_file value type
// ---------------------------------------------------------------------------

TEST_CASE("media_file default construction yields empty fields",
          "[mock][media_picker][value_type]") {
    // Arrange / Act
    media_file f;

    // Assert
    CHECK(f.full_path.empty());
    CHECK(f.file_name.empty());
    CHECK(f.content_type.empty());
}

TEST_CASE("media_file equality comparison",
          "[mock][media_picker][value_type]") {
    // Arrange
    media_file a = make_photo();
    media_file b = make_photo();
    media_file c = make_photo("other.jpg");

    // Assert
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

// ---------------------------------------------------------------------------
// media_pick_options value type
// ---------------------------------------------------------------------------

TEST_CASE("media_pick_options default construction yields empty title",
          "[mock][media_picker][value_type]") {
    // Arrange / Act
    media_pick_options opts;

    // Assert
    CHECK(opts.title.empty());
}

TEST_CASE("media_pick_options equality comparison",
          "[mock][media_picker][value_type]") {
    // Arrange
    media_pick_options a{ "Pick a photo" };
    media_pick_options b{ "Pick a photo" };
    media_pick_options c{ "Pick a video" };

    // Assert
    CHECK(a == b);
    CHECK_FALSE(a == c);
}

// ---------------------------------------------------------------------------
// Default construction
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker default state", "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Assert - capture is supported by default, no calls made yet
    CHECK(mp.is_capture_supported() == true);
    CHECK(mp.call_count() == 0);
    CHECK_FALSE(mp.last_pick_photo_options().has_value());
    CHECK_FALSE(mp.last_capture_photo_options().has_value());
    CHECK_FALSE(mp.last_pick_video_options().has_value());
    CHECK_FALSE(mp.last_capture_video_options().has_value());
}

TEST_CASE("mock_media_picker explicit capture_supported=false constructor",
          "[mock][media_picker]") {
    // Arrange / Act
    mock_media_picker mp{ false };

    // Assert
    CHECK_FALSE(mp.is_capture_supported());
    CHECK(mp.call_count() == 0);
}

// ---------------------------------------------------------------------------
// is_capture_supported + set_capture_supported
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker set_capture_supported changes the flag",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    REQUIRE(mp.is_capture_supported());

    // Act
    mp.set_capture_supported(false);

    // Assert
    CHECK_FALSE(mp.is_capture_supported());

    // Act - restore
    mp.set_capture_supported(true);

    // Assert
    CHECK(mp.is_capture_supported());
}

// ---------------------------------------------------------------------------
// pick_photo
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker pick_photo returns nullopt by default (simulates cancel)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    auto result = mp.pick_photo();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker pick_photo returns canned result",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    auto expected = make_photo();
    mp.set_pick_photo_result(expected);

    // Act
    auto result = mp.pick_photo();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker pick_photo records last options",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    media_pick_options opts{ "Choose a photo" };

    // Act
    (void)mp.pick_photo(opts);

    // Assert
    REQUIRE(mp.last_pick_photo_options().has_value());
    CHECK(mp.last_pick_photo_options()->title == "Choose a photo");
}

TEST_CASE("mock_media_picker pick_photo records default options when called with no arguments",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    (void)mp.pick_photo();

    // Assert - default-constructed options are recorded
    REQUIRE(mp.last_pick_photo_options().has_value());
    CHECK(mp.last_pick_photo_options()->title.empty());
}

TEST_CASE("mock_media_picker pick_photo overwrites last options on repeated calls",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    (void)mp.pick_photo(media_pick_options{ "first" });
    (void)mp.pick_photo(media_pick_options{ "second" });

    // Assert - most recent call wins
    REQUIRE(mp.last_pick_photo_options().has_value());
    CHECK(mp.last_pick_photo_options()->title == "second");
    CHECK(mp.call_count() == 2);
}

// ---------------------------------------------------------------------------
// capture_photo
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker capture_photo returns nullopt by default (simulates cancel)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    auto result = mp.capture_photo();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker capture_photo returns canned result when supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    auto expected = make_photo("capture.jpg");
    mp.set_capture_photo_result(expected);

    // Act
    auto result = mp.capture_photo();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("mock_media_picker capture_photo returns nullopt when capture not supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    mp.set_capture_photo_result(make_photo("capture.jpg"));

    // Act - canned result should be ignored since capture unsupported
    auto result = mp.capture_photo();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker capture_photo records last options",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    media_pick_options opts{ "Take a photo" };

    // Act
    (void)mp.capture_photo(opts);

    // Assert
    REQUIRE(mp.last_capture_photo_options().has_value());
    CHECK(mp.last_capture_photo_options()->title == "Take a photo");
}

TEST_CASE("mock_media_picker capture_photo records options even when unsupported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    media_pick_options opts{ "Camera" };

    // Act
    (void)mp.capture_photo(opts);

    // Assert - options are still recorded
    REQUIRE(mp.last_capture_photo_options().has_value());
    CHECK(mp.last_capture_photo_options()->title == "Camera");
}

// ---------------------------------------------------------------------------
// pick_video
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker pick_video returns nullopt by default (simulates cancel)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    auto result = mp.pick_video();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker pick_video returns canned result",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    auto expected = make_video();
    mp.set_pick_video_result(expected);

    // Act
    auto result = mp.pick_video();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("mock_media_picker pick_video records last options",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    media_pick_options opts{ "Choose a video" };

    // Act
    (void)mp.pick_video(opts);

    // Assert
    REQUIRE(mp.last_pick_video_options().has_value());
    CHECK(mp.last_pick_video_options()->title == "Choose a video");
}

TEST_CASE("mock_media_picker pick_video records default options when called with no arguments",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    (void)mp.pick_video();

    // Assert
    REQUIRE(mp.last_pick_video_options().has_value());
    CHECK(mp.last_pick_video_options()->title.empty());
}

// ---------------------------------------------------------------------------
// capture_video
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker capture_video returns nullopt by default (simulates cancel)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    auto result = mp.capture_video();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker capture_video returns canned result when supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    auto expected = make_video("capture.mp4");
    mp.set_capture_video_result(expected);

    // Act
    auto result = mp.capture_video();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("mock_media_picker capture_video returns nullopt when capture not supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    mp.set_capture_video_result(make_video("capture.mp4"));

    // Act
    auto result = mp.capture_video();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mp.call_count() == 1);
}

TEST_CASE("mock_media_picker capture_video records last options",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    media_pick_options opts{ "Record a video" };

    // Act
    (void)mp.capture_video(opts);

    // Assert
    REQUIRE(mp.last_capture_video_options().has_value());
    CHECK(mp.last_capture_video_options()->title == "Record a video");
}

TEST_CASE("mock_media_picker capture_video records options even when unsupported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    media_pick_options opts{ "Camera video" };

    // Act
    (void)mp.capture_video(opts);

    // Assert
    REQUIRE(mp.last_capture_video_options().has_value());
    CHECK(mp.last_capture_video_options()->title == "Camera video");
}

// ---------------------------------------------------------------------------
// call_count accumulates across all methods
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker call_count accumulates across all methods",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;

    // Act
    (void)mp.pick_photo();
    (void)mp.capture_photo();
    (void)mp.pick_video();
    (void)mp.capture_video();

    // Assert
    CHECK(mp.call_count() == 4);
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker reset clears call count and canned results",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    mp.set_pick_photo_result(make_photo());
    mp.set_capture_photo_result(make_photo("cap.jpg"));
    mp.set_pick_video_result(make_video());
    mp.set_capture_video_result(make_video("cap.mp4"));
    (void)mp.pick_photo();
    (void)mp.capture_photo();
    (void)mp.pick_video();
    (void)mp.capture_video();
    REQUIRE(mp.call_count() == 4);

    // Act
    mp.reset();

    // Assert - call count cleared
    CHECK(mp.call_count() == 0);

    // Assert - canned results cleared (all return nullopt)
    CHECK_FALSE(mp.pick_photo().has_value());
    CHECK_FALSE(mp.capture_photo().has_value());
    CHECK_FALSE(mp.pick_video().has_value());
    CHECK_FALSE(mp.capture_video().has_value());
}

TEST_CASE("mock_media_picker reset clears last-options inspection state",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    (void)mp.pick_photo(media_pick_options{ "p" });
    (void)mp.capture_photo(media_pick_options{ "c" });
    (void)mp.pick_video(media_pick_options{ "v" });
    (void)mp.capture_video(media_pick_options{ "cv" });
    REQUIRE(mp.last_pick_photo_options().has_value());

    // Act
    mp.reset();

    // Assert
    CHECK_FALSE(mp.last_pick_photo_options().has_value());
    CHECK_FALSE(mp.last_capture_photo_options().has_value());
    CHECK_FALSE(mp.last_pick_video_options().has_value());
    CHECK_FALSE(mp.last_capture_video_options().has_value());
}

TEST_CASE("mock_media_picker reset preserves capture_supported flag",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    REQUIRE_FALSE(mp.is_capture_supported());

    // Act
    mp.reset();

    // Assert - support flag unchanged by reset
    CHECK_FALSE(mp.is_capture_supported());
}

// ---------------------------------------------------------------------------
// Capture support transitions at runtime
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker capture_photo becomes available after set_capture_supported(true)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ false };
    mp.set_capture_photo_result(make_photo("shot.jpg"));

    auto result_before = mp.capture_photo();
    CHECK_FALSE(result_before.has_value());

    // Act - enable capture
    mp.set_capture_supported(true);
    auto result_after = mp.capture_photo();

    // Assert
    REQUIRE(result_after.has_value());
    CHECK(result_after->file_name == "shot.jpg");
}

TEST_CASE("mock_media_picker capture_video becomes unavailable after set_capture_supported(false)",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    mp.set_capture_video_result(make_video("clip.mp4"));

    auto result_before = mp.capture_video();
    REQUIRE(result_before.has_value());

    // Act - disable capture
    mp.set_capture_supported(false);
    auto result_after = mp.capture_video();

    // Assert
    CHECK_FALSE(result_after.has_value());
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker is usable through the abstract media_picker interface",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker impl;
    impl.set_pick_photo_result(make_photo("via_iface.jpg"));
    media_picker& iface = impl;

    // Act
    auto result = iface.pick_photo();

    // Assert
    REQUIRE(result.has_value());
    CHECK(result->file_name == "via_iface.jpg");
    CHECK(iface.is_capture_supported());
}

TEST_CASE("mock_media_picker capture methods work through abstract interface",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker impl{ true };
    impl.set_capture_photo_result(make_photo("cam.jpg"));
    impl.set_capture_video_result(make_video("cam.mp4"));
    media_picker& iface = impl;

    // Act
    auto photo = iface.capture_photo();
    auto video = iface.capture_video();

    // Assert
    REQUIRE(photo.has_value());
    CHECK(photo->content_type == "image/jpeg");

    REQUIRE(video.has_value());
    CHECK(video->content_type == "video/mp4");
}

TEST_CASE("mock_media_picker capture returns nullopt through interface when unsupported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker impl{ false };
    impl.set_capture_photo_result(make_photo("should_not_appear.jpg"));
    impl.set_capture_video_result(make_video("should_not_appear.mp4"));
    media_picker& iface = impl;

    // Act
    auto photo = iface.capture_photo();
    auto video = iface.capture_video();

    // Assert
    CHECK_FALSE(iface.is_capture_supported());
    CHECK_FALSE(photo.has_value());
    CHECK_FALSE(video.has_value());
}

// ---------------------------------------------------------------------------
// Nullopt canned result simulates cancel (independent of capture support)
// ---------------------------------------------------------------------------

TEST_CASE("mock_media_picker pick_photo returns nullopt when canned result is nullopt",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    mp.set_pick_photo_result(std::nullopt);

    // Act
    auto result = mp.pick_photo();

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_media_picker pick_video returns nullopt when canned result is nullopt",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp;
    mp.set_pick_video_result(std::nullopt);

    // Act
    auto result = mp.pick_video();

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_media_picker capture_photo returns nullopt when canned result is nullopt and supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    mp.set_capture_photo_result(std::nullopt);

    // Act
    auto result = mp.capture_photo();

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_media_picker capture_video returns nullopt when canned result is nullopt and supported",
          "[mock][media_picker]") {
    // Arrange
    mock_media_picker mp{ true };
    mp.set_capture_video_result(std::nullopt);

    // Act
    auto result = mp.capture_video();

    // Assert
    CHECK_FALSE(result.has_value());
}
