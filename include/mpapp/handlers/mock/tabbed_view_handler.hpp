// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock basic_tabbed_view handler.
//
// `tab_titles` is `vector<string>` which has no std::format spelling,
// so we record the count instead of the contents — same shape as the
// basic_picker / bindable_layout mocks. `selected_index` is a plain `int`
// and binds via the standard `bind()` plumbing.

#ifndef MPAPP_HANDLERS_MOCK_TABBED_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_MOCK_TABBED_VIEW_HANDLER_HPP

#include <cstddef>
#include <string>
#include <vector>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_tabbed_view.hpp"
#include "handler_base.hpp"

namespace mpapp::internal {

template <>
class tabbed_view_handler<platform::mock>
    : public mock_handler_base {
public:
    tabbed_view_handler() = default;

    void map_tab_titles(basic_tabbed_view& t) {
        record("tab_titles.count", t.tab_titles.get().size());
        t.tab_titles.changed.subscribe(tab_titles_slot_, tab_titles_cb_);
    }

    void map_selected_index(basic_tabbed_view& t) {
        bind("selected_index", t.selected_index, binding_selected_);
    }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_tabbed_view& /*x*/) noexcept {}


private:
    struct tab_titles_cb_t {
        tabbed_view_handler<platform::mock>* self;
        void operator()(const std::vector<std::string>& v) const {
            self->record("tab_titles.count", v.size());
        }
    };

    tab_titles_cb_t                              tab_titles_cb_{this};
    signal_slot<std::vector<std::string> const&> tab_titles_slot_{};
    detail::property_binding<int>                binding_selected_{};
};

} // namespace mpapp::internal
#endif // MPAPP_HANDLERS_MOCK_TABBED_VIEW_HANDLER_HPP
