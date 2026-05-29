// SPDX-License-Identifier: Apache-2.0
// Compile-only smoke for the RFC-0007 binding headers under Android NDK.
#include <memory>
#include <string>

#include <mpapp/binding/binding.hpp>
#include <mpapp/binding/binding_context.hpp>
#include <mpapp/binding/multi_binding.hpp>
#include <mpapp/binding/relative_source.hpp>
#include <mpapp/layout.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/view.hpp>

namespace {
class smoke_view : public mpapp::view {};
class smoke_layout : public mpapp::layout {};
struct ctx { int x = 0; };
} // namespace

int main() {
    mpapp::Observable<int>         s{ 1 };
    mpapp::Observable<std::string> t{ "" };
    mpapp::binding<int, std::string> b{
        s, t, mpapp::binding_mode::one_way,
        [](const int& n) { return std::to_string(n); } };
    (void)b.mode();

    mpapp::Observable<std::string> a{ "a" }, c{ "c" }, out{ "" };
    mpapp::multi_binding<std::string, std::string, std::string> mb{
        out, [](const std::string& x, const std::string& y) { return x + y; }, a, c };

    mpapp::binding_context bc;
    bc.set(std::make_shared<ctx>());
    (void)bc.get<ctx>();

    smoke_layout root;
    smoke_view   leaf;
    root.add(leaf);
    root.set_binding_context(std::make_shared<ctx>());
    (void)mpapp::effective_binding_context(leaf).has_value();
    (void)mpapp::find_ancestor<smoke_layout>(leaf);
    (void)mpapp::resolve_relative_source(leaf, mpapp::relative_source_mode::self);
    return 0;
}
