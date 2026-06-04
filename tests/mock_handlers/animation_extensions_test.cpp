// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0010 fluent animation extensions
// (AnimationExtensions: this.Animate / this.CancelAnimations).

#include <memory>
#include <string>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <mpapp/animation/animation_extensions.hpp>
#include <mpapp/animation/easing.hpp>
#include <mpapp/observable.hpp>

using namespace mpapp;
using Catch::Approx;

TEST_CASE("builder queues steps and reports its count",
          "[mock][animation][extensions]") {
    // Arrange
    animation_builder b;

    // Act
    b.add([](double) {}).add([](double) {});

    // Assert
    CHECK(b.step_count() == 2);
}

TEST_CASE("a 2-step fluent animation drives both steps on seek",
          "[mock][animation][extensions]") {
    // Arrange — two evenly-distributed steps over the 0..1 timeline.
    double a = -1.0;
    double c = -1.0;
    animation_builder b;
    b.add([&a](double v) { a = v; })   // owns [0.0, 0.5]
     .add([&c](double v) { c = v; });  // owns [0.5, 1.0]
    auto handle = b.build();

    CHECK(handle->step_count() == 2);
    CHECK(handle->status() == animation_status::pending);

    // Act — seek to the very end so both steps are fully driven.
    handle->seek(1.0);

    // Assert — both ticks reached their local 1.0.
    CHECK(a == Approx(1.0));
    CHECK(c == Approx(1.0));
    CHECK(handle->finished());
    CHECK(handle->status() == animation_status::completed);
}

TEST_CASE("seek to the midpoint drives the first step and starts the second",
          "[mock][animation][extensions]") {
    // Arrange
    double first  = -1.0;
    double second = -1.0;
    animation_builder b;
    b.add([&first](double v) { first = v; })   // [0.0, 0.5]
     .add([&second](double v) { second = v; }); // [0.5, 1.0]
    auto handle = b.build();

    // Act — global 0.5 is the boundary: step 1 done, step 2 at its start.
    handle->seek(0.5);

    // Assert
    CHECK(first == Approx(1.0));   // first slice complete
    CHECK(second == Approx(0.0));  // second slice exactly at begin
    CHECK(handle->status() == animation_status::running);
    CHECK_FALSE(handle->finished());
}

TEST_CASE("explicit sub-ranges are honoured and eased",
          "[mock][animation][extensions]") {
    // Arrange — one step over [0,1] with a cubic_in curve.
    double v = -1.0;
    animation_builder b;
    b.add_at(0.0, 1.0, [&v](double x) { v = x; }, easing_kind::cubic_in);
    auto handle = b.build();

    // Act — global 0.5 -> local 0.5 -> cubic_in(0.5) == 0.125.
    handle->seek(0.5);

    // Assert
    CHECK(v == Approx(0.125));
}

TEST_CASE("seek is clamped to 0..1", "[mock][animation][extensions]") {
    // Arrange
    double v = -1.0;
    animation_builder b;
    b.add_at(0.0, 1.0, [&v](double x) { v = x; });
    auto handle = b.build();

    // Act
    handle->seek(5.0);  // over-range clamps to 1.0

    // Assert
    CHECK(v == Approx(1.0));
    CHECK(handle->finished());
}

TEST_CASE("a builder with no steps still builds an empty handle",
          "[mock][animation][extensions]") {
    // Arrange
    animation_builder b;

    // Act
    auto handle = b.build();
    handle->run();

    // Assert
    CHECK(handle->step_count() == 0);
    CHECK(handle->finished());
}

TEST_CASE("run() drives the animation straight to the end",
          "[mock][animation][extensions]") {
    // Arrange
    double v = -1.0;
    animation_builder b;
    b.add([&v](double x) { v = x; });
    auto handle = b.build();

    // Act
    handle->run();

    // Assert
    CHECK(v == Approx(1.0));
}

TEST_CASE("cancel_animations cancels a handle and makes seek inert",
          "[mock][animation][extensions]") {
    // Arrange
    double v = -1.0;
    animation_builder b;
    b.add([&v](double x) { v = x; });
    auto handle = b.build();

    // Act
    const bool cancelled = cancel_animations(handle);
    handle->seek(1.0);  // must NOT drive the tick anymore

    // Assert
    CHECK(cancelled);
    CHECK(handle->cancelled());
    CHECK(handle->status() == animation_status::cancelled);
    CHECK(v == Approx(-1.0));  // untouched: the seed never ran
}

TEST_CASE("cancel is idempotent and refuses to cancel completed handles",
          "[mock][animation][extensions]") {
    // Arrange
    animation_builder b;
    b.add([](double) {});
    auto running = b.build();
    auto completed = b.build();
    completed->run();

    // Act + Assert — second cancel returns false (already cancelled).
    CHECK(running->cancel());
    CHECK_FALSE(running->cancel());
    // A completed animation cannot be cancelled.
    CHECK_FALSE(completed->cancel());
    CHECK(completed->finished());
}

TEST_CASE("cancel_animations on a null handle is safe",
          "[mock][animation][extensions]") {
    // Arrange
    std::shared_ptr<animation_handle> none;

    // Act + Assert
    CHECK_FALSE(cancel_animations(none));
}

TEST_CASE("registry registers + cancels a named animation by owner",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int owner = 0;  // any object's address works as the opaque key
    animation_builder b;
    double v = -1.0;
    b.add([&v](double x) { v = x; });
    auto handle = b.build();

    animation_registry reg;
    reg.register_animation(&owner, "fade", handle);

    // Assert registration
    CHECK(reg.has(&owner, "fade"));
    CHECK(reg.count(&owner) == 1);
    CHECK(reg.owner_count() == 1);
    CHECK(reg.find(&owner, "fade") == handle);

    // Act — cancel by (owner, name) removes it + cancels the handle.
    const bool removed = reg.cancel(&owner, "fade");

    // Assert
    CHECK(removed);
    CHECK(handle->cancelled());
    CHECK_FALSE(reg.has(&owner, "fade"));
    CHECK(reg.count(&owner) == 0);
    CHECK(reg.owner_count() == 0);  // last entry for owner pruned
}

TEST_CASE("registry cancel(owner) cancels every animation for that owner",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int owner = 0;
    animation_builder b;
    b.add([](double) {});
    auto a1 = b.build();
    auto a2 = b.build();

    animation_registry reg;
    reg.register_animation(&owner, "a", a1);
    reg.register_animation(&owner, "b", a2);
    CHECK(reg.count(&owner) == 2);

    // Act
    const std::size_t removed = reg.cancel(&owner);

    // Assert
    CHECK(removed == 2);
    CHECK(a1->cancelled());
    CHECK(a2->cancelled());
    CHECK(reg.count(&owner) == 0);
    CHECK(reg.owner_count() == 0);
}

TEST_CASE("registry replace cancels the previous animation under the same name",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int owner = 0;
    animation_builder b;
    b.add([](double) {});
    auto first  = b.build();
    auto second = b.build();

    animation_registry reg;
    reg.register_animation(&owner, "spin", first);

    // Act — registering the same (owner,name) replaces + cancels the old one.
    reg.register_animation(&owner, "spin", second);

    // Assert
    CHECK(first->cancelled());
    CHECK_FALSE(second->cancelled());
    CHECK(reg.find(&owner, "spin") == second);
    CHECK(reg.count(&owner) == 1);
}

TEST_CASE("registry ignores null owner or null handle",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int owner = 0;
    animation_builder b;
    b.add([](double) {});
    auto handle = b.build();
    animation_registry reg;

    // Act — both of these are no-ops.
    reg.register_animation(nullptr, "x", handle);
    reg.register_animation(&owner, "y", nullptr);

    // Assert
    CHECK(reg.owner_count() == 0);
    CHECK_FALSE(reg.has(&owner, "y"));
}

TEST_CASE("registry queries on absent owners/names return empty results",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int present = 0;
    int absent  = 0;
    animation_builder b;
    b.add([](double) {});
    animation_registry reg;
    reg.register_animation(&present, "here", b.build());

    // Act + Assert — absent owner
    CHECK(reg.find(&absent, "here") == nullptr);
    CHECK_FALSE(reg.has(&absent, "here"));
    CHECK(reg.count(&absent) == 0);
    CHECK_FALSE(reg.cancel(&absent, "here"));
    CHECK(reg.cancel(&absent) == 0);

    // Absent name on a present owner.
    CHECK(reg.find(&present, "missing") == nullptr);
    CHECK_FALSE(reg.cancel(&present, "missing"));
}

TEST_CASE("registry clear drops all owners",
          "[mock][animation][extensions][registry]") {
    // Arrange
    int o1 = 0;
    int o2 = 0;
    animation_builder b;
    b.add([](double) {});
    animation_registry reg;
    reg.register_animation(&o1, "a", b.build());
    reg.register_animation(&o2, "b", b.build());
    CHECK(reg.owner_count() == 2);

    // Act
    reg.clear();

    // Assert
    CHECK(reg.owner_count() == 0);
}

TEST_CASE("fluent steps can drive an Observable end-to-end",
          "[mock][animation][extensions][observable]") {
    // Arrange — wire a step into an Observable<double>'s setter.
    Observable<double> opacity{ 0.0 };
    animation_builder b;
    b.add_at(0.0, 1.0, [&opacity](double v) { opacity.set(v); });
    auto handle = b.build();

    // Act
    handle->seek(0.5);

    // Assert — Observable tracked the eased (linear) progress.
    CHECK(opacity.get() == Approx(0.5));
    handle->run();
    CHECK(opacity.get() == Approx(1.0));
}
