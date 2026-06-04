// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::share (RFC-0013 Essentials).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/share.hpp>

using namespace mpapp;

// Helper: build a share_file by member assignment (avoids aggregate
// brace-init inside std::initializer_list, which triggers a g++ codegen
// edge-case on Windows with static linkage).
static share_file make_file(std::string path, std::string content_type) {
    share_file f;
    f.path         = std::move(path);
    f.content_type = std::move(content_type);
    return f;
}

// Helper: build a share_text_request by member assignment.
static share_text_request make_text(std::string title, std::string subject,
                                    std::string text,  std::string uri) {
    share_text_request r;
    r.title   = std::move(title);
    r.subject = std::move(subject);
    r.text    = std::move(text);
    r.uri     = std::move(uri);
    return r;
}

// ---------------------------------------------------------------------------
// Default-constructed state
// ---------------------------------------------------------------------------

TEST_CASE("mock_share starts with no recorded requests",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;

    // Act / Assert - nothing called yet
    CHECK(s.request_count() == 0);
    CHECK_FALSE(s.last_text_request().has_value());
    CHECK_FALSE(s.last_file_request().has_value());
}

// ---------------------------------------------------------------------------
// share_text_request - basic round-trip
// ---------------------------------------------------------------------------

TEST_CASE("mock_share records a share_text_request",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    auto req = make_text("My Title", "My Subject", "Hello world", "https://example.com");

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_text_request().has_value());
    const auto& recorded = *s.last_text_request();
    CHECK(recorded.title   == "My Title");
    CHECK(recorded.subject == "My Subject");
    CHECK(recorded.text    == "Hello world");
    CHECK(recorded.uri     == "https://example.com");
    CHECK(s.request_count() == 1);
    CHECK_FALSE(s.last_file_request().has_value());
}

TEST_CASE("mock_share overwrites last_text_request on each call",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    auto first  = make_text("First",  "", "text one", "");
    auto second = make_text("Second", "", "text two", "");

    // Act
    s.request(first);
    s.request(second);

    // Assert - only the most recent request is retained
    REQUIRE(s.last_text_request().has_value());
    CHECK(s.last_text_request()->title == "Second");
    CHECK(s.last_text_request()->text  == "text two");
    CHECK(s.request_count() == 2);
}

TEST_CASE("mock_share accepts share_text_request with empty optional fields",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_text_request req;
    req.text = "Just text";

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_text_request().has_value());
    CHECK(s.last_text_request()->title   == "");
    CHECK(s.last_text_request()->subject == "");
    CHECK(s.last_text_request()->text    == "Just text");
    CHECK(s.last_text_request()->uri     == "");
}

TEST_CASE("mock_share accepts share_text_request with uri only",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_text_request req;
    req.uri = "https://mpapp.dev/article";

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_text_request().has_value());
    CHECK(s.last_text_request()->uri  == "https://mpapp.dev/article");
    CHECK(s.last_text_request()->text == "");
}

// ---------------------------------------------------------------------------
// share_file_request - basic round-trip
// ---------------------------------------------------------------------------

TEST_CASE("mock_share records a share_file_request with one file",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_file_request req;
    req.title = "Share Image";
    req.files.push_back(make_file("/tmp/photo.png", "image/png"));

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_file_request().has_value());
    const auto& recorded = *s.last_file_request();
    CHECK(recorded.title == "Share Image");
    REQUIRE(recorded.files.size() == 1u);
    CHECK(recorded.files[0].path         == "/tmp/photo.png");
    CHECK(recorded.files[0].content_type == "image/png");
    CHECK(s.request_count() == 1);
    CHECK_FALSE(s.last_text_request().has_value());
}

TEST_CASE("mock_share records a share_file_request with multiple files",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_file_request req;
    req.title = "Batch Share";
    req.files.push_back(make_file("/tmp/a.pdf", "application/pdf"));
    req.files.push_back(make_file("/tmp/b.jpg", "image/jpeg"));
    req.files.push_back(make_file("/tmp/c.txt", "text/plain"));

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_file_request().has_value());
    const auto& recorded = *s.last_file_request();
    CHECK(recorded.files.size() == 3u);
    CHECK(recorded.files[0].path == "/tmp/a.pdf");
    CHECK(recorded.files[1].path == "/tmp/b.jpg");
    CHECK(recorded.files[2].path == "/tmp/c.txt");
    CHECK(recorded.files[2].content_type == "text/plain");
}

TEST_CASE("mock_share records a share_file_request with no files (empty list)",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_file_request req;
    req.title = "Empty";

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_file_request().has_value());
    CHECK(s.last_file_request()->files.empty());
    CHECK(s.request_count() == 1);
}

TEST_CASE("mock_share overwrites last_file_request on each call",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_file_request r1;
    r1.title = "First";
    r1.files.push_back(make_file("/a", "text/plain"));

    share_file_request r2;
    r2.title = "Second";
    r2.files.push_back(make_file("/b", "image/png"));
    r2.files.push_back(make_file("/c", "image/png"));

    // Act
    s.request(r1);
    s.request(r2);

    // Assert
    REQUIRE(s.last_file_request().has_value());
    CHECK(s.last_file_request()->title == "Second");
    CHECK(s.last_file_request()->files.size() == 2u);
    CHECK(s.request_count() == 2);
}

// ---------------------------------------------------------------------------
// Mixed overload calls - request_count accumulates across both overloads
// ---------------------------------------------------------------------------

TEST_CASE("mock_share request_count accumulates across both overloads",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;

    // Act - interleave text and file requests
    s.request(make_text("T1", "", "text", ""));
    s.request(share_file_request{});
    s.request(make_text("T2", "", "more", ""));

    // Assert
    CHECK(s.request_count() == 3);
    REQUIRE(s.last_text_request().has_value());
    CHECK(s.last_text_request()->title == "T2");
    REQUIRE(s.last_file_request().has_value());   // set from the middle call
}

TEST_CASE("mock_share last_file_request is still set after subsequent text request",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    share_file_request fr;
    fr.title = "file share";
    fr.files.push_back(make_file("/x.png", "image/png"));

    // Act
    s.request(fr);
    s.request(make_text("text share", "", "hi", ""));

    // Assert - both slots populated independently
    REQUIRE(s.last_file_request().has_value());
    REQUIRE(s.last_text_request().has_value());
    CHECK(s.last_file_request()->title == "file share");
    CHECK(s.last_text_request()->title == "text share");
    CHECK(s.request_count() == 2);
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_share reset() clears all recorded state",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;
    s.request(make_text("title", "subj", "body", "uri"));
    share_file_request fr;
    fr.files.push_back(make_file("/f", "text/plain"));
    s.request(fr);
    REQUIRE(s.request_count() == 2);
    REQUIRE(s.last_text_request().has_value());
    REQUIRE(s.last_file_request().has_value());

    // Act
    s.reset();

    // Assert
    CHECK(s.request_count() == 0);
    CHECK_FALSE(s.last_text_request().has_value());
    CHECK_FALSE(s.last_file_request().has_value());
}

TEST_CASE("mock_share reset() on fresh instance is idempotent",
          "[mock][essentials][share]") {
    // Arrange
    mock_share s;

    // Act
    s.reset();

    // Assert
    CHECK(s.request_count() == 0);
    CHECK_FALSE(s.last_text_request().has_value());
    CHECK_FALSE(s.last_file_request().has_value());
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_share is usable through the abstract share interface (text)",
          "[mock][essentials][share]") {
    // Arrange
    mock_share impl;
    share& iface = impl;

    // Act
    iface.request(make_text("via interface", "", "hello", ""));

    // Assert via concrete type
    REQUIRE(impl.last_text_request().has_value());
    CHECK(impl.last_text_request()->title == "via interface");
    CHECK(impl.request_count() == 1);
}

TEST_CASE("mock_share is usable through the abstract share interface (files)",
          "[mock][essentials][share]") {
    // Arrange
    mock_share impl;
    share& iface = impl;
    share_file_request req;
    req.title = "interface files";
    req.files.push_back(make_file("/img.jpg", "image/jpeg"));

    // Act
    iface.request(req);

    // Assert via concrete type
    REQUIRE(impl.last_file_request().has_value());
    CHECK(impl.last_file_request()->title == "interface files");
    CHECK(impl.request_count() == 1);
}

// ---------------------------------------------------------------------------
// Value type equality (share_text_request, share_file, share_file_request)
// ---------------------------------------------------------------------------

TEST_CASE("share_text_request equality operator works correctly",
          "[essentials][share][value_type]") {
    auto a = make_text("t", "s", "body", "u");
    auto b = a;
    CHECK(a == b);
    b.text = "different";
    CHECK_FALSE(a == b);
}

TEST_CASE("share_file equality operator works correctly",
          "[essentials][share][value_type]") {
    auto f1 = make_file("/path/file.txt", "text/plain");
    auto f2 = f1;
    CHECK(f1 == f2);
    f2.content_type = "application/octet-stream";
    CHECK_FALSE(f1 == f2);
}

TEST_CASE("share_file_request equality operator works correctly",
          "[essentials][share][value_type]") {
    share_file_request r1;
    r1.title = "T";
    r1.files.push_back(make_file("/a", "image/png"));
    auto r2 = r1;
    CHECK(r1 == r2);
    r2.title = "Other";
    CHECK_FALSE(r1 == r2);
}

// ---------------------------------------------------------------------------
// share_file content_type edge cases
// ---------------------------------------------------------------------------

TEST_CASE("share_file allows empty content_type",
          "[essentials][share][value_type]") {
    // Arrange
    mock_share s;
    share_file_request req;
    req.files.push_back(make_file("/data.bin", ""));

    // Act
    s.request(req);

    // Assert
    REQUIRE(s.last_file_request().has_value());
    CHECK(s.last_file_request()->files[0].content_type.empty());
}
