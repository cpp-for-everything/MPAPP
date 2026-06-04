// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the MVVM observable_object (ObservableObject).

#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/mvvm/observable_object.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

namespace {

// A minimal derived view-model with two backing fields, exposing the
// protected helpers and the virtual hook for verification.
class person_vm : public observable_object {
public:
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] int age() const noexcept { return age_; }

    bool set_name(std::string value) {
        return set_property(name_, std::move(value), "name");
    }
    bool set_age(int value) { return set_property(age_, value, "age"); }

    // Re-broadcast a manual change for a computed property.
    void touch(std::string_view name) { raise_property_changed(name); }

    // Records every name routed through the on_property_changed hook.
    std::vector<std::string> hook_names;

protected:
    void on_property_changed(std::string_view name) override {
        hook_names.emplace_back(name);
    }

private:
    std::string name_{};
    int         age_ = 0;
};

} // namespace

TEST_CASE("set_property returns false and emits nothing on equal value",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;
    int changing = 0;
    int changed  = 0;
    signal_slot<std::string_view> s_changing;
    signal_slot<std::string_view> s_changed;
    auto on_changing = [&changing](std::string_view) { ++changing; };
    auto on_changed  = [&changed](std::string_view) { ++changed; };
    vm.property_changing.subscribe(s_changing, on_changing);
    vm.property_changed.subscribe(s_changed, on_changed);

    // Act - field starts empty; setting empty is a no-op.
    const bool result = vm.set_name("");

    // Assert
    CHECK_FALSE(result);
    CHECK(changing == 0);
    CHECK(changed == 0);
    CHECK(vm.hook_names.empty());
    CHECK(vm.name().empty());
}

TEST_CASE("set_property returns true and emits changing+changed on change",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;
    std::vector<std::string> changing_names;
    std::vector<std::string> changed_names;
    signal_slot<std::string_view> s_changing;
    signal_slot<std::string_view> s_changed;
    auto on_changing = [&changing_names](std::string_view n) {
        changing_names.emplace_back(n);
    };
    auto on_changed = [&changed_names](std::string_view n) {
        changed_names.emplace_back(n);
    };
    vm.property_changing.subscribe(s_changing, on_changing);
    vm.property_changed.subscribe(s_changed, on_changed);

    // Act
    const bool result = vm.set_name("Ada");

    // Assert
    CHECK(result);
    CHECK(vm.name() == "Ada");
    REQUIRE(changing_names.size() == 1);
    REQUIRE(changed_names.size() == 1);
    CHECK(changing_names.front() == "name");
    CHECK(changed_names.front() == "name");
}

TEST_CASE("set_property emits changing strictly before changed",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;
    std::vector<std::string> order;
    signal_slot<std::string_view> s_changing;
    signal_slot<std::string_view> s_changed;
    auto on_changing = [&order](std::string_view) {
        order.emplace_back("changing");
    };
    auto on_changed = [&order](std::string_view) {
        order.emplace_back("changed");
    };
    vm.property_changing.subscribe(s_changing, on_changing);
    vm.property_changed.subscribe(s_changed, on_changed);

    // Act
    vm.set_age(42);

    // Assert
    REQUIRE(order.size() == 2);
    CHECK(order[0] == "changing");
    CHECK(order[1] == "changed");
}

TEST_CASE("set_property works independently per field with correct names",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;
    std::vector<std::string> changed_names;
    signal_slot<std::string_view> s_changed;
    auto on_changed = [&changed_names](std::string_view n) {
        changed_names.emplace_back(n);
    };
    vm.property_changed.subscribe(s_changed, on_changed);

    // Act
    CHECK(vm.set_name("Grace"));
    CHECK(vm.set_age(7));
    CHECK_FALSE(vm.set_age(7)); // unchanged -> no emit

    // Assert
    CHECK(vm.name() == "Grace");
    CHECK(vm.age() == 7);
    REQUIRE(changed_names.size() == 2);
    CHECK(changed_names[0] == "name");
    CHECK(changed_names[1] == "age");
}

TEST_CASE("on_property_changed hook fires only on real changes",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;

    // Act
    CHECK(vm.set_name("Linus"));
    CHECK_FALSE(vm.set_name("Linus")); // equal -> no hook
    CHECK(vm.set_age(1));

    // Assert
    REQUIRE(vm.hook_names.size() == 2);
    CHECK(vm.hook_names[0] == "name");
    CHECK(vm.hook_names[1] == "age");
}

TEST_CASE("raise_property_changed emits changed and runs the hook",
          "[mock][mvvm]") {
    // Arrange
    person_vm vm;
    std::vector<std::string> changed_names;
    int changing = 0;
    signal_slot<std::string_view> s_changed;
    signal_slot<std::string_view> s_changing;
    auto on_changed = [&changed_names](std::string_view n) {
        changed_names.emplace_back(n);
    };
    auto on_changing = [&changing](std::string_view) { ++changing; };
    vm.property_changed.subscribe(s_changed, on_changed);
    vm.property_changing.subscribe(s_changing, on_changing);

    // Act
    vm.touch("full_name");

    // Assert - changed fires, changing does not, hook records the name.
    REQUIRE(changed_names.size() == 1);
    CHECK(changed_names.front() == "full_name");
    CHECK(changing == 0);
    REQUIRE(vm.hook_names.size() == 1);
    CHECK(vm.hook_names.front() == "full_name");
}

TEST_CASE("observable_object base hook is a no-op by default", "[mock][mvvm]") {
    // Arrange - a derived type that does NOT override on_property_changed,
    // exercising the base-class default implementation.
    struct plain_vm : observable_object {
        int value = 0;
        bool set_value(int v) { return set_property(value, v, "value"); }
    };
    plain_vm vm;
    int changed = 0;
    signal_slot<std::string_view> s_changed;
    auto on_changed = [&changed](std::string_view) { ++changed; };
    vm.property_changed.subscribe(s_changed, on_changed);

    // Act
    const bool result = vm.set_value(5);

    // Assert
    CHECK(result);
    CHECK(vm.value == 5);
    CHECK(changed == 1);
}
