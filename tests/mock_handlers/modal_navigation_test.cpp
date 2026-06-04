// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the modal navigation surface on
// `mpapp::application` and the underlying `detail::modal_stack` engine.

#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/application.hpp>
#include <mpapp/detail/modal_stack.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {

// Minimal concrete application - modal navigation is independent of the
// platform handler, so the in-process application surface is enough.
class test_app : public application {
public:
    void on_launch() override {}
};

class plain_view : public view {};

} // namespace

TEST_CASE("application starts with an empty modal stack",
          "[mock][modal]") {
    test_app app;
    CHECK(app.modal_stack_depth() == 0);
    CHECK(app.current_modal() == nullptr);
}

TEST_CASE("push_modal grows the stack and updates current_modal",
          "[mock][modal]") {
    test_app app;
    plain_view a, b;

    app.push_modal(a);
    CHECK(app.modal_stack_depth() == 1);
    CHECK(app.current_modal() == static_cast<view*>(&a));

    app.push_modal(b);
    CHECK(app.modal_stack_depth() == 2);
    CHECK(app.current_modal() == static_cast<view*>(&b));
}

TEST_CASE("pop_modal removes the top in LIFO order and returns it",
          "[mock][modal]") {
    test_app app;
    plain_view a, b;
    app.push_modal(a);
    app.push_modal(b);

    view* popped = app.pop_modal();
    CHECK(popped == static_cast<view*>(&b));
    CHECK(app.modal_stack_depth() == 1);
    CHECK(app.current_modal() == static_cast<view*>(&a));

    popped = app.pop_modal();
    CHECK(popped == static_cast<view*>(&a));
    CHECK(app.modal_stack_depth() == 0);
    CHECK(app.current_modal() == nullptr);
}

TEST_CASE("pop_modal on an empty stack returns nullptr",
          "[mock][modal]") {
    test_app app;
    CHECK(app.pop_modal() == nullptr);
    CHECK(app.modal_stack_depth() == 0);
}

TEST_CASE("modal_pushed / modal_popped signals fire with the page",
          "[mock][modal]") {
    test_app app;
    plain_view a;

    std::vector<view*> pushed_log;
    std::vector<view*> popped_log;
    auto on_push = [&](view* v) { pushed_log.push_back(v); };
    auto on_pop  = [&](view* v) { popped_log.push_back(v); };

    mpapp::signal<view*>::slot_type push_slot;
    mpapp::signal<view*>::slot_type pop_slot;
    app.modal_pushed().subscribe(push_slot, on_push);
    app.modal_popped().subscribe(pop_slot, on_pop);

    app.push_modal(a);
    REQUIRE(pushed_log.size() == 1);
    CHECK(pushed_log[0] == static_cast<view*>(&a));
    CHECK(popped_log.empty());

    app.pop_modal();
    REQUIRE(popped_log.size() == 1);
    CHECK(popped_log[0] == static_cast<view*>(&a));
}

TEST_CASE("modal_stack engine ignores a null page push",
          "[mock][modal]") {
    detail::modal_stack stack;

    int push_count = 0;
    auto on_push = [&](view*) { ++push_count; };
    mpapp::signal<view*>::slot_type slot;
    stack.modal_pushed.subscribe(slot, on_push);

    stack.push(nullptr);
    CHECK(stack.depth() == 0);
    CHECK(stack.empty());
    CHECK(push_count == 0);
}

TEST_CASE("modal_stack engine exposes pages() and current() directly",
          "[mock][modal]") {
    detail::modal_stack stack;
    plain_view a, b;

    CHECK(stack.empty());
    CHECK(stack.current() == nullptr);

    stack.push(&a);
    stack.push(&b);
    CHECK_FALSE(stack.empty());
    CHECK(stack.current() == static_cast<view*>(&b));

    const auto& pages = stack.pages();
    REQUIRE(pages.size() == 2);
    CHECK(pages[0] == static_cast<view*>(&a));
    CHECK(pages[1] == static_cast<view*>(&b));

    CHECK(stack.pop() == static_cast<view*>(&b));
}

TEST_CASE("modal_stack accessor on application reaches the engine",
          "[mock][modal]") {
    test_app app;
    plain_view a;

    app.modal_stack().push(&a);
    CHECK(app.modal_stack_depth() == 1);

    const application& const_app = app;
    CHECK(const_app.modal_stack().depth() == 1);
    CHECK(const_app.modal_stack().current() == static_cast<view*>(&a));
}
