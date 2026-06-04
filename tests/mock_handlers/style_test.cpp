// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for RFC-0005 style application
// (mpapp/resources/style.hpp) — covers the based_on chain, null-setter skip,
// and the swallow-setter-exceptions contract (both std and non-std throws).

#include <memory>
#include <stdexcept>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/resources/style.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {
struct sv : view {
    sv() = default;
};
} // namespace

TEST_CASE("style.apply_to runs based_on first, then own setters", "[mock][style]") {
    sv v;
    int order = 0;
    int parent_ran = 0;
    int child_ran = 0;

    auto parent = std::make_shared<style>("Button");
    parent->setters["a"] = [&](view&) { parent_ran = ++order; };

    style child{ "Button" };
    child.based_on = parent;
    child.setters["b"] = [&](view&) { child_ran = ++order; };

    child.apply_to(v);
    CHECK(parent_ran == 1);   // based_on first
    CHECK(child_ran == 2);    // own setters after
}

TEST_CASE("style.apply_to skips null setters and swallows setter exceptions",
          "[mock][style]") {
    sv v;
    style s{ "Button" };
    int ran = 0;

    s.setters["ok"]       = [&](view&) { ++ran; };
    s.setters["null"]     = nullptr;                                       // skipped
    s.setters["throw_std"]= [](view&) { throw std::runtime_error("boom"); }; // caught (std)
    s.setters["throw_any"]= [](view&) { throw 42; };                       // caught (non-std)

    CHECK_NOTHROW(s.apply_to(v));   // a bad setter never escapes apply_to
    CHECK(ran == 1);                // the valid setter still ran
}
