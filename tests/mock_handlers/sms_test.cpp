// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for the RFC-0013 Essentials `sms` surface.

#include <stdexcept>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/sms.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// sms_message value type
// ---------------------------------------------------------------------------

TEST_CASE("sms_message default-constructs to empty body and no recipients",
          "[mock][sms][sms_message]") {
    // Arrange / Act
    sms_message msg;

    // Assert
    CHECK(msg.body.empty());
    CHECK(msg.recipients.empty());
}

TEST_CASE("sms_message equality reflects body and recipients",
          "[mock][sms][sms_message]") {
    // Arrange
    sms_message a;
    a.body       = "Hello";
    a.recipients = {"alice", "bob"};

    sms_message b = a;

    // Act / Assert — equal copies
    CHECK(a == b);

    // Mutate body → no longer equal
    b.body = "Hi";
    CHECK_FALSE(a == b);

    // Restore body, mutate recipients
    b.body       = "Hello";
    b.recipients = {"alice"};
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------
// mock_sms — default (supported) behaviour
// ---------------------------------------------------------------------------

TEST_CASE("mock_sms starts supported with zero compose count and no last message",
          "[mock][sms]") {
    // Arrange / Act
    mock_sms m;

    // Assert
    CHECK(m.is_supported());
    CHECK(m.compose_count() == 0);
    CHECK_FALSE(m.last_message().has_value());
}

TEST_CASE("mock_sms::compose() with no args records empty sms_message",
          "[mock][sms]") {
    // Arrange
    mock_sms m;

    // Act
    m.compose();

    // Assert
    CHECK(m.compose_count() == 1);
    REQUIRE(m.last_message().has_value());
    CHECK(m.last_message()->body.empty());
    CHECK(m.last_message()->recipients.empty());
}

TEST_CASE("mock_sms::compose(message) records the supplied message",
          "[mock][sms]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.body       = "Test body";
    msg.recipients = {"+1234567890", "+0987654321"};

    // Act
    m.compose(msg);

    // Assert
    CHECK(m.compose_count() == 1);
    REQUIRE(m.last_message().has_value());
    CHECK(m.last_message()->body       == "Test body");
    CHECK(m.last_message()->recipients == std::vector<std::string>{"+1234567890", "+0987654321"});
}

TEST_CASE("mock_sms::compose_count increments with each call",
          "[mock][sms]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.body = "msg";

    // Act
    m.compose();
    m.compose(msg);
    m.compose(msg);

    // Assert
    CHECK(m.compose_count() == 3);
}

TEST_CASE("mock_sms::last_message reflects the most recent call",
          "[mock][sms]") {
    // Arrange
    mock_sms m;

    sms_message first;
    first.body = "first";

    sms_message second;
    second.body = "second";
    second.recipients = {"x"};

    // Act
    m.compose(first);
    CHECK(m.last_message()->body == "first");

    m.compose(second);

    // Assert — last_message is now the second
    REQUIRE(m.last_message().has_value());
    CHECK(m.last_message()->body       == "second");
    CHECK(m.last_message()->recipients == std::vector<std::string>{"x"});
}

// ---------------------------------------------------------------------------
// mock_sms — reset()
// ---------------------------------------------------------------------------

TEST_CASE("mock_sms::reset clears count and last_message but keeps supported flag",
          "[mock][sms]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.body = "before reset";
    m.compose(msg);
    REQUIRE(m.compose_count() == 1);
    REQUIRE(m.last_message().has_value());

    // Act
    m.reset();

    // Assert — state cleared
    CHECK(m.compose_count() == 0);
    CHECK_FALSE(m.last_message().has_value());

    // supported_ flag unchanged (default true)
    CHECK(m.is_supported());
}

TEST_CASE("mock_sms::reset on unsupported mock preserves unsupported flag",
          "[mock][sms]") {
    // Arrange
    mock_sms m;
    m.set_supported(false);

    // Act
    m.reset();

    // Assert — supported flag still false after reset
    CHECK_FALSE(m.is_supported());
    CHECK(m.compose_count() == 0);
    CHECK_FALSE(m.last_message().has_value());
}

// ---------------------------------------------------------------------------
// mock_sms — not-supported path
// ---------------------------------------------------------------------------

TEST_CASE("mock_sms::compose() throws when not supported",
          "[mock][sms][not_supported]") {
    // Arrange
    mock_sms m;
    m.set_supported(false);

    // Act / Assert
    CHECK_THROWS_AS(m.compose(), std::runtime_error);
    CHECK(m.compose_count() == 0);
    CHECK_FALSE(m.last_message().has_value());
}

TEST_CASE("mock_sms::compose(message) throws when not supported",
          "[mock][sms][not_supported]") {
    // Arrange
    mock_sms m;
    m.set_supported(false);
    sms_message msg;
    msg.body       = "body";
    msg.recipients = {"a"};

    // Act / Assert
    CHECK_THROWS_AS(m.compose(msg), std::runtime_error);
    CHECK(m.compose_count() == 0);
    CHECK_FALSE(m.last_message().has_value());
}

TEST_CASE("mock_sms can be toggled between supported and unsupported",
          "[mock][sms]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.body = "hi";

    // Act — supported → call succeeds
    m.compose(msg);
    CHECK(m.compose_count() == 1);

    // Act — disable support → subsequent call throws
    m.set_supported(false);
    CHECK_THROWS_AS(m.compose(msg), std::runtime_error);
    CHECK(m.compose_count() == 1);   // count unchanged after throw

    // Act — re-enable → call succeeds again
    m.set_supported(true);
    m.compose(msg);
    CHECK(m.compose_count() == 2);
}

// ---------------------------------------------------------------------------
// sms abstract interface via mock_sms (polymorphic usage)
// ---------------------------------------------------------------------------

TEST_CASE("sms interface can be used via base pointer",
          "[mock][sms][interface]") {
    // Arrange
    mock_sms concrete;
    sms* iface = &concrete;

    sms_message msg;
    msg.body       = "via interface";
    msg.recipients = {"recipient"};

    // Act
    iface->compose();
    iface->compose(msg);

    // Assert — inspect through the concrete mock
    CHECK(concrete.compose_count() == 2);
    REQUIRE(concrete.last_message().has_value());
    CHECK(concrete.last_message()->body == "via interface");
}

// ---------------------------------------------------------------------------
// sms_message with empty recipients list
// ---------------------------------------------------------------------------

TEST_CASE("sms_message with body but no recipients is valid and records correctly",
          "[mock][sms][sms_message]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.body = "broadcast with no explicit recipient";

    // Act
    m.compose(msg);

    // Assert
    REQUIRE(m.last_message().has_value());
    CHECK(m.last_message()->body       == "broadcast with no explicit recipient");
    CHECK(m.last_message()->recipients.empty());
}

// ---------------------------------------------------------------------------
// sms_message with empty body
// ---------------------------------------------------------------------------

TEST_CASE("sms_message with recipients but empty body records correctly",
          "[mock][sms][sms_message]") {
    // Arrange
    mock_sms m;
    sms_message msg;
    msg.recipients = {"+1"};

    // Act
    m.compose(msg);

    // Assert
    REQUIRE(m.last_message().has_value());
    CHECK(m.last_message()->body.empty());
    CHECK(m.last_message()->recipients == std::vector<std::string>{"+1"});
}
