// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Catch2 tests for mpapp::contacts / mock_contacts.

#include <optional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/contacts.hpp>

using namespace mpapp;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static contact make_contact(std::string display, std::string given,
                             std::string family,
                             std::vector<std::string> emails = {},
                             std::vector<std::string> phones = {}) {
    contact c;
    c.display_name = std::move(display);
    c.given_name   = std::move(given);
    c.family_name  = std::move(family);
    c.emails       = std::move(emails);
    c.phones       = std::move(phones);
    return c;
}

// ---------------------------------------------------------------------------
// contact value-type tests
// ---------------------------------------------------------------------------

TEST_CASE("contact default construction yields empty fields",
          "[mock][essentials][contacts][contact]") {
    // Arrange / Act
    contact c;

    // Assert
    CHECK(c.display_name.empty());
    CHECK(c.given_name.empty());
    CHECK(c.family_name.empty());
    CHECK(c.emails.empty());
    CHECK(c.phones.empty());
}

TEST_CASE("contact equality compares all fields",
          "[mock][essentials][contacts][contact]") {
    // Arrange
    contact a = make_contact("Ada Lovelace", "Ada", "Lovelace",
                              {"ada@example.com"}, {"+1-555-0001"});
    contact b = a;

    // Act / Assert — identical copies are equal
    CHECK(a == b);

    // Mutate each field and verify inequality
    b.display_name = "X";
    CHECK_FALSE(a == b);
    b = a;

    b.given_name = "X";
    CHECK_FALSE(a == b);
    b = a;

    b.family_name = "X";
    CHECK_FALSE(a == b);
    b = a;

    b.emails = {};
    CHECK_FALSE(a == b);
    b = a;

    b.phones = {};
    CHECK_FALSE(a == b);
}

TEST_CASE("contact copy and assignment", "[mock][essentials][contacts][contact]") {
    // Arrange
    contact src = make_contact("Grace Hopper", "Grace", "Hopper",
                                {"grace@example.com"}, {"+1-555-0002"});

    // Act
    contact copy = src;                // copy-ctor
    contact assigned;
    assigned = src;                    // copy-assign

    // Assert
    CHECK(copy == src);
    CHECK(assigned == src);
}

// ---------------------------------------------------------------------------
// mock_contacts: get_all()
// ---------------------------------------------------------------------------

TEST_CASE("mock_contacts get_all returns empty list by default",
          "[mock][essentials][contacts][get_all]") {
    // Arrange
    mock_contacts mc;

    // Act
    auto result = mc.get_all();

    // Assert
    CHECK(result.empty());
}

TEST_CASE("mock_contacts get_all returns the canned list after set_contacts",
          "[mock][essentials][contacts][get_all]") {
    // Arrange
    mock_contacts mc;
    contact c1 = make_contact("Ada Lovelace", "Ada", "Lovelace",
                               {"ada@example.com"}, {"+1-555-0001"});
    contact c2 = make_contact("Grace Hopper", "Grace", "Hopper",
                               {"grace@example.com"}, {"+1-555-0002"});

    // Act
    mc.set_contacts({c1, c2});
    auto result = mc.get_all();

    // Assert
    REQUIRE(result.size() == 2);
    CHECK(result[0] == c1);
    CHECK(result[1] == c2);
}

TEST_CASE("mock_contacts get_all can be replaced with a new list",
          "[mock][essentials][contacts][get_all]") {
    // Arrange
    mock_contacts mc;
    contact c1 = make_contact("Ada", "Ada", "L");
    mc.set_contacts({c1});

    // Act — replace
    contact c2 = make_contact("Grace", "Grace", "H");
    mc.set_contacts({c2});
    auto result = mc.get_all();

    // Assert — only the new list is present
    REQUIRE(result.size() == 1);
    CHECK(result[0] == c2);
}

TEST_CASE("mock_contacts get_all can be cleared by setting an empty list",
          "[mock][essentials][contacts][get_all]") {
    // Arrange
    mock_contacts mc;
    mc.set_contacts({make_contact("Ada", "Ada", "L")});

    // Act
    mc.set_contacts({});
    auto result = mc.get_all();

    // Assert
    CHECK(result.empty());
}

TEST_CASE("mock_contacts get_all is callable through base-class pointer",
          "[mock][essentials][contacts][get_all]") {
    // Arrange
    mock_contacts mc;
    mc.set_contacts({make_contact("Alan Turing", "Alan", "Turing")});
    contacts* base = &mc;

    // Act
    auto result = base->get_all();

    // Assert
    REQUIRE(result.size() == 1);
    CHECK(result[0].display_name == "Alan Turing");
}

// ---------------------------------------------------------------------------
// mock_contacts: pick()
// ---------------------------------------------------------------------------

TEST_CASE("mock_contacts pick returns nullopt by default (not supported / cancel)",
          "[mock][essentials][contacts][pick]") {
    // Arrange
    mock_contacts mc;

    // Act
    auto result = mc.pick();

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_contacts pick returns canned contact after set_pick_result",
          "[mock][essentials][contacts][pick]") {
    // Arrange
    mock_contacts mc;
    contact expected = make_contact("Ada Lovelace", "Ada", "Lovelace",
                                    {"ada@example.com"}, {"+1-555-0001"});
    mc.set_pick_result(expected);

    // Act
    auto result = mc.pick();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

TEST_CASE("mock_contacts pick can be reset to nullopt after being set",
          "[mock][essentials][contacts][pick]") {
    // Arrange
    mock_contacts mc;
    mc.set_pick_result(make_contact("Ada", "Ada", "L"));

    // Act — reset to cancel
    mc.set_pick_result(std::nullopt);
    auto result = mc.pick();

    // Assert
    CHECK_FALSE(result.has_value());
}

TEST_CASE("mock_contacts pick is callable through base-class pointer",
          "[mock][essentials][contacts][pick]") {
    // Arrange
    mock_contacts mc;
    contact expected = make_contact("Grace Hopper", "Grace", "Hopper");
    mc.set_pick_result(expected);
    contacts* base = &mc;

    // Act
    auto result = base->pick();

    // Assert
    REQUIRE(result.has_value());
    CHECK(*result == expected);
}

// ---------------------------------------------------------------------------
// mock_contacts: pick_count()
// ---------------------------------------------------------------------------

TEST_CASE("mock_contacts pick_count starts at zero",
          "[mock][essentials][contacts][pick_count]") {
    // Arrange / Act
    mock_contacts mc;

    // Assert
    CHECK(mc.pick_count() == 0);
}

TEST_CASE("mock_contacts pick_count increments on every pick() call",
          "[mock][essentials][contacts][pick_count]") {
    // Arrange
    mock_contacts mc;

    // Act / Assert
    mc.pick();
    CHECK(mc.pick_count() == 1);

    mc.pick();
    CHECK(mc.pick_count() == 2);

    mc.pick();
    CHECK(mc.pick_count() == 3);
}

TEST_CASE("mock_contacts pick_count increments even when pick returns nullopt",
          "[mock][essentials][contacts][pick_count]") {
    // Arrange
    mock_contacts mc; // default: nullopt

    // Act
    mc.pick();
    mc.pick();

    // Assert
    CHECK(mc.pick_count() == 2);
}

// ---------------------------------------------------------------------------
// Integration: combined get_all + pick workflow
// ---------------------------------------------------------------------------

TEST_CASE("mock_contacts supports combined get_all and pick workflow",
          "[mock][essentials][contacts][integration]") {
    // Arrange
    mock_contacts mc;
    contact ada   = make_contact("Ada Lovelace", "Ada", "Lovelace",
                                  {"ada@example.com"}, {"+1-555-0001"});
    contact grace = make_contact("Grace Hopper", "Grace", "Hopper",
                                  {"grace@example.com"}, {"+1-555-0002"});
    mc.set_contacts({ada, grace});
    mc.set_pick_result(grace);

    // Act
    auto all    = mc.get_all();
    auto picked = mc.pick();

    // Assert
    REQUIRE(all.size() == 2);
    REQUIRE(picked.has_value());
    CHECK(*picked == grace);
    CHECK(mc.pick_count() == 1);
}

// ---------------------------------------------------------------------------
// contact field content tests
// ---------------------------------------------------------------------------

TEST_CASE("contact supports multiple email addresses and phone numbers",
          "[mock][essentials][contacts][contact]") {
    // Arrange
    contact c;
    c.display_name = "Multi Contact";
    c.emails       = {"a@x.com", "b@x.com", "c@x.com"};
    c.phones       = {"+1-555-1111", "+1-555-2222"};

    // Act
    mock_contacts mc;
    mc.set_contacts({c});
    auto result = mc.get_all();

    // Assert
    REQUIRE(result.size() == 1);
    CHECK(result[0].emails.size() == 3);
    CHECK(result[0].phones.size() == 2);
    CHECK(result[0].emails[1] == "b@x.com");
    CHECK(result[0].phones[0] == "+1-555-1111");
}

TEST_CASE("contact with no emails or phones is valid",
          "[mock][essentials][contacts][contact]") {
    // Arrange
    contact c = make_contact("Name Only", "Name", "Only");

    // Act
    mock_contacts mc;
    mc.set_contacts({c});
    auto result = mc.get_all();

    // Assert
    REQUIRE(result.size() == 1);
    CHECK(result[0].emails.empty());
    CHECK(result[0].phones.empty());
}
