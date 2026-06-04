// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 WebAuthenticator.

#include <map>
#include <optional>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/web_authenticator.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// web_authenticator_result
// ---------------------------------------------------------------------------

TEST_CASE("web_authenticator_result: get returns value for present key",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    web_authenticator_result result;
    result.properties["access_token"] = "tok_abc";
    result.properties["id_token"]     = "id_xyz";

    // Act + Assert
    REQUIRE(result.get("access_token").has_value());
    CHECK(*result.get("access_token") == "tok_abc");

    REQUIRE(result.get("id_token").has_value());
    CHECK(*result.get("id_token") == "id_xyz");
}

TEST_CASE("web_authenticator_result: get returns nullopt for absent key",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    web_authenticator_result result;
    result.properties["code"] = "auth_code";

    // Act + Assert
    CHECK_FALSE(result.get("access_token").has_value());
    CHECK_FALSE(result.get("missing_key").has_value());
}

TEST_CASE("web_authenticator_result: access_token convenience returns token when present",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    web_authenticator_result result;
    result.properties["access_token"] = "Bearer eyJhbGci";

    // Act + Assert
    REQUIRE(result.access_token().has_value());
    CHECK(*result.access_token() == "Bearer eyJhbGci");
}

TEST_CASE("web_authenticator_result: access_token returns nullopt when absent",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    web_authenticator_result result;
    result.properties["code"] = "some_code";

    // Act + Assert
    CHECK_FALSE(result.access_token().has_value());
}

TEST_CASE("web_authenticator_result: default-constructed has empty properties",
          "[mock][essentials][web_authenticator]") {
    // Arrange + Act
    web_authenticator_result result;

    // Assert
    CHECK(result.properties.empty());
    CHECK_FALSE(result.get("anything").has_value());
    CHECK_FALSE(result.access_token().has_value());
}

// ---------------------------------------------------------------------------
// web_authenticator_options
// ---------------------------------------------------------------------------

TEST_CASE("web_authenticator_options: default-constructed has expected defaults",
          "[mock][essentials][web_authenticator]") {
    // Arrange + Act
    web_authenticator_options opts;

    // Assert
    CHECK(opts.url.empty());
    CHECK(opts.callback_url.empty());
    CHECK(opts.prefers_ephemeral == false);
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - initial state
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: initial state has no calls and returns nullopt",
          "[mock][essentials][web_authenticator]") {
    // Arrange + Act
    mock_web_authenticator mock;

    // Assert
    CHECK(mock.call_count() == 0);
    CHECK_FALSE(mock.last_options().has_value());
}

TEST_CASE("mock_web_authenticator: authenticate returns nullopt by default (not set)",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts;
    opts.url          = "https://auth.example.com/oauth";
    opts.callback_url = "myapp://callback";

    // Act
    auto result = mock.authenticate(opts);

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mock.call_count() == 1);
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - set_result with optional
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: set_result(optional) - success path returns canned result",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_result canned;
    canned.properties["access_token"] = "tok_123";
    canned.properties["expires_in"]   = "3600";
    mock.set_result(canned);

    web_authenticator_options opts;
    opts.url          = "https://login.example.com/authorize";
    opts.callback_url = "myapp://auth";

    // Act
    auto result = mock.authenticate(opts);

    // Assert
    REQUIRE(result.has_value());
    REQUIRE(result->get("access_token").has_value());
    CHECK(*result->get("access_token") == "tok_123");
    REQUIRE(result->get("expires_in").has_value());
    CHECK(*result->get("expires_in") == "3600");
    CHECK(mock.call_count() == 1);
}

TEST_CASE("mock_web_authenticator: set_result(nullopt) simulates cancellation",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    mock.set_result(std::optional<web_authenticator_result>{});

    web_authenticator_options opts;
    opts.url          = "https://auth.example.com/oauth";
    opts.callback_url = "myapp://callback";

    // Act
    auto result = mock.authenticate(opts);

    // Assert
    CHECK_FALSE(result.has_value());
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - set_result with map (convenience overload)
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: set_result(map) convenience overload works",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    mock.set_result(std::map<std::string, std::string>{
        { "access_token", "map_tok" },
        { "token_type",   "Bearer"  },
    });

    web_authenticator_options opts;
    opts.url          = "https://idp.example.org/token";
    opts.callback_url = "app://redirect";

    // Act
    auto result = mock.authenticate(opts);

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result->access_token() == "map_tok");
    REQUIRE(result->get("token_type").has_value());
    CHECK(*result->get("token_type") == "Bearer");
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - set_not_supported
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: set_not_supported returns nullopt",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    // First prime with a real result, then switch to not-supported to
    // verify the override works.
    mock.set_result(std::map<std::string, std::string>{ { "access_token", "x" } });
    mock.set_not_supported();

    web_authenticator_options opts;
    opts.url          = "https://auth.example.com";
    opts.callback_url = "app://cb";

    // Act
    auto result = mock.authenticate(opts);

    // Assert
    CHECK_FALSE(result.has_value());
    CHECK(mock.call_count() == 1);
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - last_options recording
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: last_options records the options passed",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    mock.set_not_supported();

    web_authenticator_options opts;
    opts.url                = "https://accounts.google.com/oauth";
    opts.callback_url       = "com.example.app://oauth2redirect";
    opts.prefers_ephemeral  = true;

    // Act
    (void)mock.authenticate(opts);

    // Assert
    REQUIRE(mock.last_options().has_value());
    CHECK(mock.last_options()->url           == "https://accounts.google.com/oauth");
    CHECK(mock.last_options()->callback_url  == "com.example.app://oauth2redirect");
    CHECK(mock.last_options()->prefers_ephemeral == true);
}

TEST_CASE("mock_web_authenticator: last_options records prefers_ephemeral false",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts;
    opts.url                = "https://auth.example.com";
    opts.callback_url       = "app://cb";
    opts.prefers_ephemeral  = false;

    // Act
    (void)mock.authenticate(opts);

    // Assert
    REQUIRE(mock.last_options().has_value());
    CHECK(mock.last_options()->prefers_ephemeral == false);
}

TEST_CASE("mock_web_authenticator: last_options updated on repeated calls",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts1;
    opts1.url          = "https://first.example.com";
    opts1.callback_url = "app://first";

    web_authenticator_options opts2;
    opts2.url          = "https://second.example.com";
    opts2.callback_url = "app://second";

    // Act
    (void)mock.authenticate(opts1);
    (void)mock.authenticate(opts2);

    // Assert
    CHECK(mock.call_count() == 2);
    REQUIRE(mock.last_options().has_value());
    CHECK(mock.last_options()->url == "https://second.example.com");
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - call_count
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: call_count increments on each authenticate()",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts;
    opts.url          = "https://auth.example.com";
    opts.callback_url = "app://cb";

    // Act + Assert
    CHECK(mock.call_count() == 0);
    (void)mock.authenticate(opts);
    CHECK(mock.call_count() == 1);
    (void)mock.authenticate(opts);
    CHECK(mock.call_count() == 2);
    (void)mock.authenticate(opts);
    CHECK(mock.call_count() == 3);
}

// ---------------------------------------------------------------------------
// mock_web_authenticator - reset_calls
// ---------------------------------------------------------------------------

TEST_CASE("mock_web_authenticator: reset_calls clears call_count and last_options",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts;
    opts.url          = "https://auth.example.com";
    opts.callback_url = "app://cb";
    (void)mock.authenticate(opts);
    REQUIRE(mock.call_count() == 1);
    REQUIRE(mock.last_options().has_value());

    // Act
    mock.reset_calls();

    // Assert
    CHECK(mock.call_count() == 0);
    CHECK_FALSE(mock.last_options().has_value());
}

TEST_CASE("mock_web_authenticator: reset_calls does not affect canned result",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    mock.set_result(std::map<std::string, std::string>{ { "access_token", "persistent" } });
    web_authenticator_options opts;
    opts.url          = "https://auth.example.com";
    opts.callback_url = "app://cb";
    (void)mock.authenticate(opts);

    // Act
    mock.reset_calls();

    // Assert: canned result survives reset
    auto result = mock.authenticate(opts);
    REQUIRE(result.has_value());
    CHECK(*result->access_token() == "persistent");
    CHECK(mock.call_count() == 1); // only the post-reset call
}

// ---------------------------------------------------------------------------
// Interface polymorphism: verify mock satisfies the abstract interface
// ---------------------------------------------------------------------------

TEST_CASE("web_authenticator: mock satisfies abstract interface via pointer",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator concrete;
    concrete.set_result(std::map<std::string, std::string>{ { "access_token", "poly_tok" } });
    web_authenticator* iface = &concrete;

    web_authenticator_options opts;
    opts.url          = "https://auth.example.com/oauth";
    opts.callback_url = "myapp://auth";

    // Act
    auto result = iface->authenticate(opts);

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result->access_token() == "poly_tok");
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST_CASE("web_authenticator_result: multiple properties accessible independently",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    web_authenticator_result result;
    result.properties["access_token"]  = "access";
    result.properties["refresh_token"] = "refresh";
    result.properties["id_token"]      = "id";
    result.properties["expires_in"]    = "7200";
    result.properties["token_type"]    = "Bearer";

    // Act + Assert
    CHECK(*result.get("access_token")  == "access");
    CHECK(*result.get("refresh_token") == "refresh");
    CHECK(*result.get("id_token")      == "id");
    CHECK(*result.get("expires_in")    == "7200");
    CHECK(*result.get("token_type")    == "Bearer");
    CHECK_FALSE(result.get("scope").has_value());
}

TEST_CASE("mock_web_authenticator: empty-URL options are still recorded",
          "[mock][essentials][web_authenticator]") {
    // Arrange
    mock_web_authenticator mock;
    web_authenticator_options opts; // all defaults

    // Act
    (void)mock.authenticate(opts);

    // Assert
    REQUIRE(mock.last_options().has_value());
    CHECK(mock.last_options()->url.empty());
    CHECK(mock.last_options()->callback_url.empty());
    CHECK(mock.last_options()->prefers_ephemeral == false);
}
