// SPDX-License-Identifier: Apache-2.0
// Compile-only smoke test for the RFC-0005 + RFC-0006 mock headers
// under Android NDK clang. Pulls in every new header MPAPP shipped
// this session + instantiates each public type so deferred template
// errors fire at this TU rather than silently slipping through.

// Note: mpapp/handlers/mock/resource_recorder.hpp pulls in
// mock_handler_base which uses <format> — Android NDK's libc++ on r26
// doesn't ship <format>, and the mock recorder is host-side test
// helper anyway. Skip it here; we still exercise every real-side
// resource/VSM type below.
#include <memory>
#include <string>

#include <mpapp/resources/resource_dictionary.hpp>
#include <mpapp/resources/static_resource.hpp>
#include <mpapp/resources/style.hpp>
#include <mpapp/resources/visual_state_manager.hpp>
#include <mpapp/view.hpp>

namespace {

class smoke_view : public mpapp::view {
public:
    smoke_view() = default;
};

} // namespace

int main() {
    // Force instantiation of every template the headers expose so a
    // dependent-name compile error in this NDK / libc++ pairing would
    // fire here.
    mpapp::resource_dictionary d;
    d.put("k", std::string{"v"});
    auto hit = d.try_get<std::string>("k");
    (void)hit;

    smoke_view root;
    smoke_view child;
    child.set_parent(&root);
    root.resources = std::make_shared<mpapp::resource_dictionary>();
    root.resources->put("a", 1.0);
    auto ancestor_hit = mpapp::find_in<double>(child, "a");
    (void)ancestor_hit;

    mpapp::style s{"View"};
    s.setters["x"] = [](mpapp::view&) {};
    s.apply_to(child);

    mpapp::visual_state_manager vsm;
    vsm.groups.push_back(mpapp::visual_state_group{"Common"});
    vsm.groups.back().states.push_back(mpapp::visual_state{
        std::string{mpapp::visual_states::pressed}});
    (void)vsm.go_to_state(child, mpapp::visual_states::pressed);
    return 0;
}
