// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Smoke canary: the framework's PLATFORM-NEUTRAL public surface headers
// are reachable and self-consistent from a plain mpapp-core + Catch2
// translation unit — no platform SDK on the include path.
//
// This deliberately does NOT include the app umbrella <mpapp/mpapp.hpp>:
// the umbrella pulls in every wrapper (button/label/…), and after
// ADR-0024 each wrapper embeds a <platform::current> handler by value,
// which on Windows needs the WinUI 3 / WindowsAppSDK projection headers.
// "The full umbrella + real handler stack compiles" is validated by the
// example targets (which wire the per-platform SDK include paths); this
// canary stays SDK-free so it builds on a clean cloud Windows runner
// (T-0032 Path B).

#include <catch2/catch_test_macros.hpp>

// Reactive primitives (ADR-0009 template wrappers).
#include <mpapp/command.hpp>
#include <mpapp/computed.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/signal.hpp>

// Platform-neutral view/layout surface + the mock-first config
// subsystems. None of these embed a platform handler — they hold
// handler pointers or are pure configuration.
#include <mpapp/layout.hpp>
#include <mpapp/layout_types.hpp>
#include <mpapp/view.hpp>

#include <mpapp/resources/resource_dictionary.hpp>
#include <mpapp/resources/static_resource.hpp>
#include <mpapp/resources/style.hpp>
#include <mpapp/resources/visual_state_manager.hpp>

TEST_CASE("mpapp platform-neutral surface headers are reachable", "[smoke]") {
    using namespace mpapp;
    REQUIRE(true);
}
