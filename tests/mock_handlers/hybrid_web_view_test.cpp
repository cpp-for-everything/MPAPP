// SPDX-License-Identifier: Apache-2.0
// Mock-handler tests for `mpapp::hybrid_web_view`.

#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/hybrid_web_view_handler.hpp>
#include <mpapp/hybrid_web_view.hpp>

using namespace mpapp;

TEST_CASE("hybrid_web_view bridge sends and receives",
          "[mock][hybrid_web_view]") {
    hybrid_web_view h;
    CHECK(h.hybrid_namespace.get() == "mpapp");
    CHECK(h.last_message_in.get().empty());
    CHECK(h.last_message_out().empty());

    h.send_to_js("from-cpp");
    CHECK(h.last_message_out() == "from-cpp");

    h.simulate_inbound("from-js");
    CHECK(h.last_message_in.get() == "from-js");
}

TEST_CASE("mock handler records bridge traffic",
          "[mock][hybrid_web_view]") {
    hybrid_web_view h;
    hybrid_web_view_handler<platform::mock> hh;
    hh.map_messages(h);
    hh.clear_calls();

    h.simulate_inbound("hello-from-js");
    h.send_to_js("hello-from-cpp");

    REQUIRE(hh.calls_as_strings() == std::vector<std::string>{
        "message_received=hello-from-js",
        "message_sent=hello-from-cpp",
    });
}
