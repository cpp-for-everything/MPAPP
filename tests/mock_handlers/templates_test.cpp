// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the MAUI templating primitives:
//   mpapp::data_template_selector
//   mpapp::control_template
//   mpapp::content_presenter
//
// Covers every public method plus null-factory / empty-selector paths.

#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include <mpapp/templates/data_template_selector.hpp>
#include <mpapp/templates/control_template.hpp>
#include <mpapp/templates/content_presenter.hpp>

using namespace mpapp;

// -----------------------------------------------------------------------
// Minimal concrete view used as a test double for mpapp::view.
// -----------------------------------------------------------------------
namespace {

struct test_view : view {
    test_view() = default;
    int tag = 0;
};

// -----------------------------------------------------------------------
// Concrete data_template_selector
// Selects between two templates based on whether the item starts with 'A'.
// -----------------------------------------------------------------------
struct alpha_selector : data_template_selector {
    data_template tpl_alpha{ "alpha-tpl", nullptr };
    data_template tpl_other{ "other-tpl", nullptr };

    [[nodiscard]] const data_template*
    on_select_template(const std::string& item,
                       const view* /*container*/) const override {
        if (!item.empty() && item[0] == 'A') {
            return &tpl_alpha;
        }
        return &tpl_other;
    }
};

// Selector that always returns nullptr (simulates "no applicable template").
struct null_selector : data_template_selector {
    [[nodiscard]] const data_template*
    on_select_template(const std::string& /*item*/,
                       const view* /*container*/) const override {
        return nullptr;
    }
};

} // namespace

// -----------------------------------------------------------------------
// data_template_selector tests
// -----------------------------------------------------------------------

TEST_CASE("data_template_selector routes to alpha template for A-items",
          "[mock][templates][data_template_selector]") {
    // Arrange
    alpha_selector sel;
    test_view host;

    // Act
    const data_template* result = sel.select_template("Apple", &host);

    // Assert
    REQUIRE(result != nullptr);
    CHECK(result->name == "alpha-tpl");
}

TEST_CASE("data_template_selector routes to other template for non-A items",
          "[mock][templates][data_template_selector]") {
    // Arrange
    alpha_selector sel;
    test_view host;

    // Act
    const data_template* result = sel.select_template("Banana", &host);

    // Assert
    REQUIRE(result != nullptr);
    CHECK(result->name == "other-tpl");
}

TEST_CASE("data_template_selector passes null container without crashing",
          "[mock][templates][data_template_selector]") {
    // Arrange
    alpha_selector sel;

    // Act / Assert — no crash with nullptr container
    const data_template* result = sel.select_template("Avocado", nullptr);
    REQUIRE(result != nullptr);
    CHECK(result->name == "alpha-tpl");
}

TEST_CASE("data_template_selector handles empty item string",
          "[mock][templates][data_template_selector]") {
    // Arrange
    alpha_selector sel;

    // Act
    const data_template* result = sel.select_template("", nullptr);

    // Assert — falls through to other branch
    REQUIRE(result != nullptr);
    CHECK(result->name == "other-tpl");
}

TEST_CASE("data_template_selector returns nullptr when selector finds no match",
          "[mock][templates][data_template_selector]") {
    // Arrange
    null_selector sel;

    // Act
    const data_template* result = sel.select_template("anything", nullptr);

    // Assert
    CHECK(result == nullptr);
}

// -----------------------------------------------------------------------
// control_template tests
// -----------------------------------------------------------------------

TEST_CASE("control_template has_factory returns false when no factory set",
          "[mock][templates][control_template]") {
    // Arrange
    control_template ct;

    // Act / Assert
    CHECK_FALSE(ct.has_factory());
}

TEST_CASE("control_template instantiate returns nullptr when factory is empty",
          "[mock][templates][control_template]") {
    // Arrange
    control_template ct;

    // Act
    auto result = ct.instantiate();

    // Assert
    CHECK(result == nullptr);
}

TEST_CASE("control_template has_factory returns true after factory is set",
          "[mock][templates][control_template]") {
    // Arrange
    control_template ct;
    ct.factory = []() { return std::make_unique<test_view>(); };

    // Act / Assert
    CHECK(ct.has_factory());
}

TEST_CASE("control_template instantiate returns a non-null view when factory is set",
          "[mock][templates][control_template]") {
    // Arrange
    control_template ct;
    ct.factory = []() {
        auto v = std::make_unique<test_view>();
        v->tag = 42;
        return v;
    };

    // Act
    auto result = ct.instantiate();

    // Assert
    REQUIRE(result != nullptr);
    CHECK(static_cast<test_view*>(result.get())->tag == 42);
}

TEST_CASE("control_template instantiate produces a fresh view on each call",
          "[mock][templates][control_template]") {
    // Arrange
    control_template ct;
    int counter = 0;
    ct.factory = [&counter]() {
        auto v = std::make_unique<test_view>();
        v->tag = ++counter;
        return v;
    };

    // Act
    auto first  = ct.instantiate();
    auto second = ct.instantiate();

    // Assert — each call creates a distinct object
    REQUIRE(first  != nullptr);
    REQUIRE(second != nullptr);
    CHECK(first.get()  != second.get());
    CHECK(static_cast<test_view*>(first.get())->tag  == 1);
    CHECK(static_cast<test_view*>(second.get())->tag == 2);
}

// -----------------------------------------------------------------------
// content_presenter tests
// -----------------------------------------------------------------------

TEST_CASE("content_presenter default-constructed has no content or templated_child",
          "[mock][templates][content_presenter]") {
    // Arrange / Act
    content_presenter cp;

    // Assert
    CHECK(cp.content()         == nullptr);
    CHECK(cp.templated_child() == nullptr);
}

TEST_CASE("content_presenter set_content / content round-trip",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    test_view child;

    // Act
    cp.set_content(&child);

    // Assert
    CHECK(cp.content() == &child);
}

TEST_CASE("content_presenter set_content nullptr clears the child",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    test_view child;
    cp.set_content(&child);

    // Act
    cp.set_content(nullptr);

    // Assert
    CHECK(cp.content() == nullptr);
}

TEST_CASE("content_presenter apply_template stores the instantiated view",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    control_template ct;
    ct.factory = []() {
        auto v = std::make_unique<test_view>();
        v->tag = 99;
        return v;
    };

    // Act
    cp.apply_template(ct);

    // Assert
    REQUIRE(cp.templated_child() != nullptr);
    CHECK(static_cast<test_view*>(cp.templated_child())->tag == 99);
}

TEST_CASE("content_presenter apply_template with empty factory yields nullptr templated_child",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    control_template  ct;   // no factory

    // Act
    cp.apply_template(ct);

    // Assert
    CHECK(cp.templated_child() == nullptr);
}

TEST_CASE("content_presenter apply_template replaces a previous templated child",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    control_template  first_ct;
    first_ct.factory = []() {
        auto v = std::make_unique<test_view>();
        v->tag = 1;
        return v;
    };
    cp.apply_template(first_ct);
    view* first_ptr = cp.templated_child();

    control_template second_ct;
    second_ct.factory = []() {
        auto v = std::make_unique<test_view>();
        v->tag = 2;
        return v;
    };

    // Act
    cp.apply_template(second_ct);

    // Assert — new child installed, old child dropped
    REQUIRE(cp.templated_child() != nullptr);
    CHECK(cp.templated_child() != first_ptr);
    CHECK(static_cast<test_view*>(cp.templated_child())->tag == 2);
}

TEST_CASE("content_presenter set_content and apply_template are independent",
          "[mock][templates][content_presenter]") {
    // Arrange
    content_presenter cp;
    test_view         presented_child;
    cp.set_content(&presented_child);

    control_template ct;
    ct.factory = []() { return std::make_unique<test_view>(); };

    // Act
    cp.apply_template(ct);

    // Assert — both slots are populated independently
    CHECK(cp.content()         == &presented_child);
    CHECK(cp.templated_child() != nullptr);
}

TEST_CASE("content_presenter is a view subclass",
          "[mock][templates][content_presenter]") {
    // Arrange / Act
    content_presenter cp;

    // Assert — static_cast compiles + runtime check via parent pointer
    view* as_view = &cp;
    CHECK(as_view != nullptr);
}
