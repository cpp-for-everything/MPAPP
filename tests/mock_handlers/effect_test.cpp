// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0009 effect attach-point
// (mpapp/effects/effect.hpp) — exercises the base resolution_id() accessor
// and the on_attached/on_detached lifecycle hooks (both the default no-op
// bodies and a derived override).

#include <catch2/catch_test_macros.hpp>

#include <mpapp/effects/effect.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {
class test_view : public view {
public:
    test_view() = default;
};

// A derived effect that records the lifecycle callbacks, so we cover both the
// virtual dispatch and an override (the base no-ops are covered directly below).
class recording_effect : public effect {
public:
    using effect::effect;
    void on_attached(view& /*host*/) override { ++attached; }
    void on_detached(view& /*host*/) override { ++detached; }
    int attached = 0;
    int detached = 0;
};
} // namespace

TEST_CASE("effect exposes its resolution id", "[mock][effect]") {
    effect e{ "MyCompany.MyEffect" };
    CHECK(e.resolution_id() == "MyCompany.MyEffect");
}

TEST_CASE("base effect lifecycle hooks are no-op-safe", "[mock][effect]") {
    effect e{ "MyCompany.NoOp" };
    test_view host;
    // Default bodies do nothing and must not touch the host.
    e.on_attached(host);
    e.on_detached(host);
    CHECK(e.resolution_id() == "MyCompany.NoOp");
}

TEST_CASE("derived effect observes attach/detach", "[mock][effect]") {
    recording_effect fx{ "MyCompany.Recording" };
    test_view host;

    CHECK(fx.attached == 0);
    fx.on_attached(host);
    fx.on_attached(host);
    CHECK(fx.attached == 2);

    CHECK(fx.detached == 0);
    fx.on_detached(host);
    CHECK(fx.detached == 1);

    // Virtual dispatch through the base reference still hits the override.
    effect& base = fx;
    base.on_attached(host);
    CHECK(fx.attached == 3);
    CHECK(base.resolution_id() == "MyCompany.Recording");
}
