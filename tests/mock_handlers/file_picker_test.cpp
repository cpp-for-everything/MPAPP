// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::file_picker (RFC-0013 Essentials).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/file_picker.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Default-constructed state
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker starts with no recorded calls and nullopt results",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Assert - nothing called yet
    CHECK(fp.pick_call_count() == 0);
    CHECK(fp.pick_multiple_call_count() == 0);
    CHECK_FALSE(fp.last_pick_options().has_value());
    CHECK_FALSE(fp.last_pick_multiple_options().has_value());
}

// ---------------------------------------------------------------------------
// pick() - canned result not set (default: nullopt / cancelled)
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick returns nullopt when no result is set",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Act
    auto result = fp.pick();

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(fp.pick_call_count() == 1);
}

TEST_CASE("mock_file_picker pick records default options when called with no args",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Act - discard result; we only care about the recorded options
    (void)fp.pick();

    // Assert
    REQUIRE(fp.last_pick_options().has_value());
    CHECK(fp.last_pick_options()->title.empty());
    CHECK(fp.last_pick_options()->file_types.empty());
}

// ---------------------------------------------------------------------------
// pick() - canned result set
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick returns the canned file_result",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    file_result expected{ "/home/user/photo.png", "photo.png", "image/png" };
    fp.set_pick_result(expected);

    // Act
    auto result = fp.pick();

    // Assert
    REQUIRE(result.has_value());
    CHECK(result->full_path    == "/home/user/photo.png");
    CHECK(result->file_name    == "photo.png");
    CHECK(result->content_type == "image/png");
    CHECK(fp.pick_call_count() == 1);
}

TEST_CASE("mock_file_picker pick records the options passed by caller",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/a.pdf", "a.pdf", "application/pdf" });
    pick_options opts;
    opts.title      = "Select PDF";
    opts.file_types = { "application/pdf", ".pdf" };

    // Act - discard result; we only care about the recorded options
    (void)fp.pick(opts);

    // Assert
    REQUIRE(fp.last_pick_options().has_value());
    CHECK(fp.last_pick_options()->title == "Select PDF");
    REQUIRE(fp.last_pick_options()->file_types.size() == 2u);
    CHECK(fp.last_pick_options()->file_types[0] == "application/pdf");
    CHECK(fp.last_pick_options()->file_types[1] == ".pdf");
}

TEST_CASE("mock_file_picker pick call_count increments with each call",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/x", "x", "" });

    // Act
    (void)fp.pick();
    (void)fp.pick();
    (void)fp.pick();

    // Assert
    CHECK(fp.pick_call_count() == 3);
}

TEST_CASE("mock_file_picker pick overwrites last_pick_options on each call",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    pick_options first;
    first.title = "First";
    pick_options second;
    second.title = "Second";

    // Act
    (void)fp.pick(first);
    (void)fp.pick(second);

    // Assert
    REQUIRE(fp.last_pick_options().has_value());
    CHECK(fp.last_pick_options()->title == "Second");
    CHECK(fp.pick_call_count() == 2);
}

TEST_CASE("mock_file_picker pick returns same canned result on repeated calls",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/doc.txt", "doc.txt", "text/plain" });

    // Act
    auto r1 = fp.pick();
    auto r2 = fp.pick();

    // Assert - canned result is not consumed/cleared
    REQUIRE(r1.has_value());
    REQUIRE(r2.has_value());
    CHECK(r1->full_path == r2->full_path);
}

// ---------------------------------------------------------------------------
// pick() - set canned result back to nullopt (simulate cancellation)
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick returns nullopt after set_pick_result(nullopt)",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/file.txt", "file.txt", "text/plain" });
    fp.set_pick_result(std::nullopt);   // override to cancelled

    // Act
    auto result = fp.pick();

    // Assert
    CHECK_FALSE(result.has_value());
}

// ---------------------------------------------------------------------------
// pick_multiple() - default (empty / not supported)
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick_multiple returns empty vector when no results set",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Act
    auto results = fp.pick_multiple();

    // Assert
    CHECK(results.empty());
    CHECK(fp.pick_multiple_call_count() == 1);
}

TEST_CASE("mock_file_picker pick_multiple records default options when called with no args",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Act - discard result; we only care about the recorded options
    (void)fp.pick_multiple();

    // Assert
    REQUIRE(fp.last_pick_multiple_options().has_value());
    CHECK(fp.last_pick_multiple_options()->title.empty());
    CHECK(fp.last_pick_multiple_options()->file_types.empty());
}

// ---------------------------------------------------------------------------
// pick_multiple() - canned results set
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick_multiple returns the canned results",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    std::vector<file_result> canned = {
        { "/a.png", "a.png", "image/png" },
        { "/b.jpg", "b.jpg", "image/jpeg" },
        { "/c.pdf", "c.pdf", "application/pdf" },
    };
    fp.set_pick_multiple_results(canned);

    // Act
    auto results = fp.pick_multiple();

    // Assert
    REQUIRE(results.size() == 3u);
    CHECK(results[0].full_path    == "/a.png");
    CHECK(results[0].file_name    == "a.png");
    CHECK(results[0].content_type == "image/png");
    CHECK(results[1].full_path    == "/b.jpg");
    CHECK(results[2].full_path    == "/c.pdf");
    CHECK(fp.pick_multiple_call_count() == 1);
}

TEST_CASE("mock_file_picker pick_multiple records the options passed by caller",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_multiple_results({ file_result{ "/img.png", "img.png", "image/png" } });
    pick_options opts;
    opts.title      = "Select Images";
    opts.file_types = { "image/*" };

    // Act - discard result; we only care about the recorded options
    (void)fp.pick_multiple(opts);

    // Assert
    REQUIRE(fp.last_pick_multiple_options().has_value());
    CHECK(fp.last_pick_multiple_options()->title == "Select Images");
    REQUIRE(fp.last_pick_multiple_options()->file_types.size() == 1u);
    CHECK(fp.last_pick_multiple_options()->file_types[0] == "image/*");
}

TEST_CASE("mock_file_picker pick_multiple call_count increments with each call",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_multiple_results({ { "/x", "x", "" } });

    // Act
    (void)fp.pick_multiple();
    (void)fp.pick_multiple();

    // Assert
    CHECK(fp.pick_multiple_call_count() == 2);
}

TEST_CASE("mock_file_picker pick_multiple overwrites last options on each call",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    pick_options first;
    first.title = "First";
    pick_options second;
    second.title = "Second";

    // Act
    (void)fp.pick_multiple(first);
    (void)fp.pick_multiple(second);

    // Assert
    REQUIRE(fp.last_pick_multiple_options().has_value());
    CHECK(fp.last_pick_multiple_options()->title == "Second");
    CHECK(fp.pick_multiple_call_count() == 2);
}

TEST_CASE("mock_file_picker pick_multiple returns same canned results on repeated calls",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_multiple_results({ { "/f.txt", "f.txt", "text/plain" } });

    // Act
    auto r1 = fp.pick_multiple();
    auto r2 = fp.pick_multiple();

    // Assert - canned results not consumed
    REQUIRE(r1.size() == 1u);
    REQUIRE(r2.size() == 1u);
    CHECK(r1[0].full_path == r2[0].full_path);
}

TEST_CASE("mock_file_picker pick_multiple with a single file",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_multiple_results({ { "/only.doc", "only.doc", "application/msword" } });

    // Act
    auto results = fp.pick_multiple();

    // Assert
    REQUIRE(results.size() == 1u);
    CHECK(results[0].file_name == "only.doc");
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker reset() clears all recorded state and canned results",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/f.txt", "f.txt", "text/plain" });
    fp.set_pick_multiple_results({ { "/a.png", "a.png", "image/png" } });
    (void)fp.pick(pick_options{ "T", {} });
    (void)fp.pick_multiple(pick_options{ "M", {} });
    REQUIRE(fp.pick_call_count() == 1);
    REQUIRE(fp.pick_multiple_call_count() == 1);

    // Act
    fp.reset();

    // Assert
    CHECK(fp.pick_call_count() == 0);
    CHECK(fp.pick_multiple_call_count() == 0);
    CHECK_FALSE(fp.last_pick_options().has_value());
    CHECK_FALSE(fp.last_pick_multiple_options().has_value());
    CHECK_FALSE(fp.pick().has_value());              // canned single cleared
    CHECK(fp.pick_multiple().empty());               // canned multiple cleared
}

TEST_CASE("mock_file_picker reset() on fresh instance is idempotent",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;

    // Act
    fp.reset();

    // Assert
    CHECK(fp.pick_call_count() == 0);
    CHECK(fp.pick_multiple_call_count() == 0);
    CHECK_FALSE(fp.last_pick_options().has_value());
    CHECK_FALSE(fp.last_pick_multiple_options().has_value());
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker is usable through the abstract file_picker interface (pick)",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker impl;
    impl.set_pick_result(file_result{ "/via.txt", "via.txt", "text/plain" });
    file_picker& iface = impl;

    // Act
    auto result = iface.pick(pick_options{ "interface test", {} });

    // Assert via concrete type
    REQUIRE(result.has_value());
    CHECK(result->file_name == "via.txt");
    CHECK(impl.pick_call_count() == 1);
    REQUIRE(impl.last_pick_options().has_value());
    CHECK(impl.last_pick_options()->title == "interface test");
}

TEST_CASE("mock_file_picker is usable through the abstract file_picker interface (pick_multiple)",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker impl;
    impl.set_pick_multiple_results({
        { "/i1.png", "i1.png", "image/png" },
        { "/i2.png", "i2.png", "image/png" },
    });
    file_picker& iface = impl;

    // Act
    auto results = iface.pick_multiple(pick_options{ "multi interface", {} });

    // Assert via concrete type
    REQUIRE(results.size() == 2u);
    CHECK(results[0].file_name == "i1.png");
    CHECK(impl.pick_multiple_call_count() == 1);
}

// ---------------------------------------------------------------------------
// Value type equality (file_result, pick_options)
// ---------------------------------------------------------------------------

TEST_CASE("file_result equality operator works correctly",
          "[essentials][file_picker][value_type]") {
    // Arrange
    file_result a{ "/path/file.txt", "file.txt", "text/plain" };
    file_result b = a;

    // Assert equal
    CHECK(a == b);

    // Act - modify one field
    b.content_type = "application/octet-stream";

    // Assert not equal
    CHECK_FALSE(a == b);
}

TEST_CASE("file_result default-constructed fields are empty strings",
          "[essentials][file_picker][value_type]") {
    // Arrange / Act
    file_result r;

    // Assert
    CHECK(r.full_path.empty());
    CHECK(r.file_name.empty());
    CHECK(r.content_type.empty());
}

TEST_CASE("pick_options equality operator works correctly",
          "[essentials][file_picker][value_type]") {
    // Arrange
    pick_options a{ "Open File", { "image/png", ".png" } };
    pick_options b = a;

    // Assert equal
    CHECK(a == b);

    // Act
    b.title = "Different Title";

    // Assert not equal
    CHECK_FALSE(a == b);
}

TEST_CASE("pick_options default-constructed fields are empty",
          "[essentials][file_picker][value_type]") {
    // Arrange / Act
    pick_options opts;

    // Assert
    CHECK(opts.title.empty());
    CHECK(opts.file_types.empty());
}

// ---------------------------------------------------------------------------
// pick_options file_types edge cases
// ---------------------------------------------------------------------------

TEST_CASE("mock_file_picker pick forwards file_types filter in options",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(std::nullopt);
    pick_options opts;
    opts.file_types = { ".txt", ".csv", ".json" };

    // Act - discard result; testing that options are forwarded
    (void)fp.pick(opts);

    // Assert
    REQUIRE(fp.last_pick_options().has_value());
    REQUIRE(fp.last_pick_options()->file_types.size() == 3u);
    CHECK(fp.last_pick_options()->file_types[2] == ".json");
}

TEST_CASE("mock_file_picker pick_multiple forwards empty file_types in options",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    pick_options opts;
    opts.title = "All files";
    // file_types intentionally left empty → means all files

    // Act - discard result; testing that options are forwarded
    (void)fp.pick_multiple(opts);

    // Assert
    REQUIRE(fp.last_pick_multiple_options().has_value());
    CHECK(fp.last_pick_multiple_options()->file_types.empty());
    CHECK(fp.last_pick_multiple_options()->title == "All files");
}

// ---------------------------------------------------------------------------
// pick() and pick_multiple() are independent - calls don't cross-contaminate
// ---------------------------------------------------------------------------

TEST_CASE("pick and pick_multiple call counts are independent",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    fp.set_pick_result(file_result{ "/s.txt", "s.txt", "" });
    fp.set_pick_multiple_results({ { "/m1.txt", "m1.txt", "" }, { "/m2.txt", "m2.txt", "" } });

    // Act
    (void)fp.pick();
    (void)fp.pick();
    (void)fp.pick_multiple();

    // Assert
    CHECK(fp.pick_call_count() == 2);
    CHECK(fp.pick_multiple_call_count() == 1);
}

TEST_CASE("last_pick_options and last_pick_multiple_options are independent",
          "[mock][essentials][file_picker]") {
    // Arrange
    mock_file_picker fp;
    pick_options single_opts;
    single_opts.title = "Single";
    pick_options multi_opts;
    multi_opts.title = "Multi";

    // Act
    (void)fp.pick(single_opts);
    (void)fp.pick_multiple(multi_opts);

    // Assert - both recorded independently
    REQUIRE(fp.last_pick_options().has_value());
    REQUIRE(fp.last_pick_multiple_options().has_value());
    CHECK(fp.last_pick_options()->title == "Single");
    CHECK(fp.last_pick_multiple_options()->title == "Multi");
}
