// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0013 Essentials core.

#include <string>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/essentials/connectivity.hpp>
#include <mpapp/essentials/device_info.hpp>
#include <mpapp/essentials/preferences.hpp>
#include <mpapp/essentials/secure_storage.hpp>
#include <mpapp/signal.hpp>

using namespace mpapp;

TEST_CASE("preferences typed get/set + default-on-miss", "[mock][essentials][preferences]") {
    in_memory_preferences p;

    p.set("name", std::string{ "Ada" });
    p.set("count", 3L);
    p.set("ratio", 1.5);
    p.set("enabled", true);

    CHECK(p.get("name", std::string{ "?" }) == "Ada");
    CHECK(p.get("count", 0L) == 3L);
    CHECK(p.get("ratio", 0.0) == 1.5);
    CHECK(p.get("enabled", false) == true);

    // Default-on-miss.
    CHECK(p.get("missing", std::string{ "fallback" }) == "fallback");
    CHECK(p.get("missing", 99L) == 99L);

    CHECK(p.contains("name"));
    p.remove("name");
    CHECK_FALSE(p.contains("name"));

    p.clear();
    CHECK_FALSE(p.contains("count"));
}

TEST_CASE("secure_storage set/get/remove", "[mock][essentials][secure]") {
    in_memory_secure_storage s;

    s.set("token", "abc123");
    REQUIRE(s.get("token").has_value());
    CHECK(*s.get("token") == "abc123");
    CHECK(s.contains("token"));

    CHECK_FALSE(s.get("nope").has_value());

    CHECK(s.remove("token"));          // existed
    CHECK_FALSE(s.remove("token"));    // already gone
    CHECK_FALSE(s.contains("token"));

    s.set("a", "1");
    s.set("b", "2");
    s.remove_all();
    CHECK_FALSE(s.contains("a"));
    CHECK_FALSE(s.contains("b"));
}

TEST_CASE("connectivity reports access + fires on change", "[mock][essentials][connectivity]") {
    mock_connectivity c{ network_access::internet };
    CHECK(c.access() == network_access::internet);
    CHECK(c.is_online());

    network_access last = network_access::none;
    int hits = 0;
    signal_slot<network_access> slot;
    auto cb = [&](network_access a) { last = a; ++hits; };
    c.connectivity_changed.subscribe(slot, cb);

    c.set_access(network_access::none);
    CHECK(hits == 1);
    CHECK(last == network_access::none);
    CHECK_FALSE(c.is_online());

    c.set_access(network_access::none);   // same value -> no fire
    CHECK(hits == 1);

    c.set_access(network_access::internet);
    CHECK(hits == 2);
    CHECK(c.is_online());
}

TEST_CASE("device_info value type + current_device_info", "[mock][essentials][device]") {
    device_info a;
    a.platform = device_platform::linux_;
    a.idiom    = device_idiom::desktop;
    a.model    = "dev";
    device_info b = a;
    CHECK(a == b);
    b.model = "other";
    CHECK_FALSE(a == b);

    // On any supported host the compile-time platform resolves to a
    // known platform + idiom (never unknown).
    const auto info = current_device_info();
    CHECK(info.platform != device_platform::unknown);
    CHECK(info.idiom    != device_idiom::unknown);
}
