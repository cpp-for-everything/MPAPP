// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for RFC-0007 RelativeSource resolution
// (mpapp/binding/relative_source.hpp) - covers Self, the untyped
// FindAncestor branch, and the unknown-mode fallback.

#include <catch2/catch_test_macros.hpp>

#include <mpapp/binding/relative_source.hpp>
#include <mpapp/view.hpp>

using namespace mpapp;

namespace {
struct rv : view {
    rv() = default;
};
} // namespace

TEST_CASE("resolve_relative_source returns the element itself for Self mode",
          "[mock][binding][relative_source]") {
    rv v;
    CHECK(resolve_relative_source(v, relative_source_mode::self) == &v);
}

TEST_CASE("resolve_relative_source FindAncestor returns the parent (untyped)",
          "[mock][binding][relative_source]") {
    rv v;
    // A root view has no parent, so the untyped FindAncestor branch returns
    // null - but the case + return statements still execute.
    CHECK(resolve_relative_source(v, relative_source_mode::find_ancestor) == v.parent());
}

TEST_CASE("resolve_relative_source falls back to null for an unknown mode",
          "[mock][binding][relative_source]") {
    rv v;
    const auto bogus = static_cast<relative_source_mode>(99);
    CHECK(resolve_relative_source(v, bogus) == nullptr);
}
