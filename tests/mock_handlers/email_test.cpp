// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::email (RFC-0013 Essentials).

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/email.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// email_body_format to_string
// ---------------------------------------------------------------------------

TEST_CASE("email_body_format to_string covers all cases and fallback",
          "[mock][essentials][email][enum]") {
    CHECK(to_string(email_body_format::plain_text) == "plain_text");
    CHECK(to_string(email_body_format::html)       == "html");
    CHECK(to_string(static_cast<email_body_format>(99)) == "?");
}

// ---------------------------------------------------------------------------
// email_message value type
// ---------------------------------------------------------------------------

TEST_CASE("email_message default-constructs with expected defaults",
          "[essentials][email][value_type]") {
    // Arrange / Act
    email_message m;

    // Assert
    CHECK(m.to.empty());
    CHECK(m.cc.empty());
    CHECK(m.bcc.empty());
    CHECK(m.subject.empty());
    CHECK(m.body.empty());
    CHECK(m.format == email_body_format::plain_text);
    CHECK(m.attachments.empty());
}

TEST_CASE("email_message equality operator works correctly",
          "[essentials][email][value_type]") {
    // Arrange
    email_message a;
    a.to      = { "alice@example.com" };
    a.subject = "Hello";
    a.body    = "World";
    a.format  = email_body_format::html;

    email_message b = a;
    CHECK(a == b);

    b.subject = "Different";
    CHECK_FALSE(a == b);
}

TEST_CASE("email_message equality detects difference in vector fields",
          "[essentials][email][value_type]") {
    email_message a;
    a.to = { "x@x.com", "y@y.com" };
    email_message b = a;
    CHECK(a == b);

    b.cc = { "z@z.com" };
    CHECK_FALSE(a == b);
}

TEST_CASE("email_message equality detects difference in attachments",
          "[essentials][email][value_type]") {
    email_message a;
    a.attachments = { "/tmp/file.pdf" };
    email_message b = a;
    CHECK(a == b);

    b.attachments.push_back("/tmp/extra.pdf");
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// mock_email default state
// ---------------------------------------------------------------------------

TEST_CASE("mock_email starts with no recorded composes",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;

    // Assert - nothing called yet
    CHECK(e.compose_count() == 0);
    CHECK(e.blank_compose_count() == 0);
    CHECK_FALSE(e.last_message().has_value());
}

// ---------------------------------------------------------------------------
// compose() - blank overload
// ---------------------------------------------------------------------------

TEST_CASE("mock_email compose() increments compose_count and blank_compose_count",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;

    // Act
    e.compose();

    // Assert
    CHECK(e.compose_count()       == 1);
    CHECK(e.blank_compose_count() == 1);
    CHECK_FALSE(e.last_message().has_value()); // blank compose does NOT set last_message
}

TEST_CASE("mock_email compose() called multiple times accumulates counts",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;

    // Act
    e.compose();
    e.compose();
    e.compose();

    // Assert
    CHECK(e.compose_count()       == 3);
    CHECK(e.blank_compose_count() == 3);
    CHECK_FALSE(e.last_message().has_value());
}

// ---------------------------------------------------------------------------
// compose(email_message) - message overload
// ---------------------------------------------------------------------------

TEST_CASE("mock_email compose(message) records the message",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message msg;
    msg.to      = { "bob@example.com" };
    msg.subject = "Test subject";
    msg.body    = "Test body";
    msg.format  = email_body_format::plain_text;

    // Act
    e.compose(msg);

    // Assert
    REQUIRE(e.last_message().has_value());
    const auto& recorded = *e.last_message();
    CHECK(recorded.to      == std::vector<std::string>{ "bob@example.com" });
    CHECK(recorded.subject == "Test subject");
    CHECK(recorded.body    == "Test body");
    CHECK(recorded.format  == email_body_format::plain_text);
    CHECK(e.compose_count()       == 1);
    CHECK(e.blank_compose_count() == 0);
}

TEST_CASE("mock_email compose(message) records an HTML message with full fields",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message msg;
    msg.to          = { "alice@example.com", "charlie@example.com" };
    msg.cc          = { "cc@example.com" };
    msg.bcc         = { "bcc@example.com" };
    msg.subject     = "HTML Email";
    msg.body        = "<h1>Hello</h1>";
    msg.format      = email_body_format::html;
    msg.attachments = { "/tmp/doc.pdf", "/tmp/image.png" };

    // Act
    e.compose(msg);

    // Assert
    REQUIRE(e.last_message().has_value());
    const auto& rec = *e.last_message();
    CHECK(rec.to.size()          == 2u);
    CHECK(rec.to[0]              == "alice@example.com");
    CHECK(rec.to[1]              == "charlie@example.com");
    CHECK(rec.cc                 == std::vector<std::string>{ "cc@example.com" });
    CHECK(rec.bcc                == std::vector<std::string>{ "bcc@example.com" });
    CHECK(rec.subject            == "HTML Email");
    CHECK(rec.body               == "<h1>Hello</h1>");
    CHECK(rec.format             == email_body_format::html);
    CHECK(rec.attachments.size() == 2u);
    CHECK(rec.attachments[0]     == "/tmp/doc.pdf");
    CHECK(rec.attachments[1]     == "/tmp/image.png");
}

TEST_CASE("mock_email compose(message) overwrites last_message on each call",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message first;
    first.subject = "First";
    email_message second;
    second.subject = "Second";

    // Act
    e.compose(first);
    e.compose(second);

    // Assert - only the most recent message is retained
    REQUIRE(e.last_message().has_value());
    CHECK(e.last_message()->subject == "Second");
    CHECK(e.compose_count() == 2);
}

TEST_CASE("mock_email compose(message) accepts a default-constructed email_message",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message msg; // all defaults

    // Act
    e.compose(msg);

    // Assert
    REQUIRE(e.last_message().has_value());
    const auto& rec = *e.last_message();
    CHECK(rec.to.empty());
    CHECK(rec.subject.empty());
    CHECK(rec.body.empty());
    CHECK(rec.format == email_body_format::plain_text);
    CHECK(rec.attachments.empty());
}

// ---------------------------------------------------------------------------
// Mixed overload calls - compose_count accumulates across both overloads
// ---------------------------------------------------------------------------

TEST_CASE("mock_email compose_count accumulates across both overloads",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;

    // Act - interleave blank and message composes
    e.compose();
    email_message m1;
    m1.subject = "First message";
    e.compose(m1);
    e.compose();
    email_message m2;
    m2.subject = "Second message";
    e.compose(m2);

    // Assert
    CHECK(e.compose_count()       == 4);
    CHECK(e.blank_compose_count() == 2);
    REQUIRE(e.last_message().has_value());
    CHECK(e.last_message()->subject == "Second message");
}

TEST_CASE("mock_email blank_compose_count is not affected by compose(message)",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message m;
    m.subject = "msg";

    // Act
    e.compose(m);
    e.compose(m);

    // Assert
    CHECK(e.compose_count()       == 2);
    CHECK(e.blank_compose_count() == 0);
}

TEST_CASE("mock_email last_message is still set after subsequent blank compose",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message m;
    m.subject = "keep me";

    // Act
    e.compose(m);
    e.compose(); // blank - should NOT wipe last_message_

    // Assert
    REQUIRE(e.last_message().has_value());
    CHECK(e.last_message()->subject == "keep me");
    CHECK(e.compose_count()       == 2);
    CHECK(e.blank_compose_count() == 1);
}

// ---------------------------------------------------------------------------
// reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_email reset() clears all recorded state",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    e.compose();
    email_message m;
    m.subject = "clear me";
    e.compose(m);
    REQUIRE(e.compose_count() == 2);
    REQUIRE(e.last_message().has_value());

    // Act
    e.reset();

    // Assert
    CHECK(e.compose_count()       == 0);
    CHECK(e.blank_compose_count() == 0);
    CHECK_FALSE(e.last_message().has_value());
}

TEST_CASE("mock_email reset() on a fresh instance is idempotent",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;

    // Act
    e.reset();

    // Assert
    CHECK(e.compose_count()       == 0);
    CHECK(e.blank_compose_count() == 0);
    CHECK_FALSE(e.last_message().has_value());
}

// ---------------------------------------------------------------------------
// Abstract interface polymorphism
// ---------------------------------------------------------------------------

TEST_CASE("mock_email is usable through the abstract email interface (blank)",
          "[mock][essentials][email]") {
    // Arrange
    mock_email impl;
    email& iface = impl;

    // Act
    iface.compose();

    // Assert via concrete type
    CHECK(impl.compose_count()       == 1);
    CHECK(impl.blank_compose_count() == 1);
    CHECK_FALSE(impl.last_message().has_value());
}

TEST_CASE("mock_email is usable through the abstract email interface (message)",
          "[mock][essentials][email]") {
    // Arrange
    mock_email impl;
    email& iface = impl;
    email_message m;
    m.to      = { "test@example.com" };
    m.subject = "via interface";

    // Act
    iface.compose(m);

    // Assert via concrete type
    REQUIRE(impl.last_message().has_value());
    CHECK(impl.last_message()->subject == "via interface");
    CHECK(impl.compose_count() == 1);
}

// ---------------------------------------------------------------------------
// Edge cases: empty recipient lists, multiple recipients, no attachments
// ---------------------------------------------------------------------------

TEST_CASE("mock_email compose records message with multiple to/cc/bcc recipients",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message m;
    m.to  = { "a@a.com", "b@b.com", "c@c.com" };
    m.cc  = { "d@d.com" };
    m.bcc = { "e@e.com", "f@f.com" };

    // Act
    e.compose(m);

    // Assert
    REQUIRE(e.last_message().has_value());
    CHECK(e.last_message()->to.size()  == 3u);
    CHECK(e.last_message()->cc.size()  == 1u);
    CHECK(e.last_message()->bcc.size() == 2u);
}

TEST_CASE("mock_email compose records message with no attachments",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message m;
    m.to      = { "x@x.com" };
    m.subject = "No attachments";

    // Act
    e.compose(m);

    // Assert
    REQUIRE(e.last_message().has_value());
    CHECK(e.last_message()->attachments.empty());
}

TEST_CASE("mock_email compose records message with single attachment",
          "[mock][essentials][email]") {
    // Arrange
    mock_email e;
    email_message m;
    m.attachments = { "/home/user/report.pdf" };

    // Act
    e.compose(m);

    // Assert
    REQUIRE(e.last_message().has_value());
    REQUIRE(e.last_message()->attachments.size() == 1u);
    CHECK(e.last_message()->attachments[0] == "/home/user/report.pdf");
}
