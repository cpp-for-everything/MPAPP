// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/SearchBar.md
//
// `mpapp::search_bar` — single-line text input affordance with
// search-affordance styling (search icon + dedicated search-key on the
// IME). The C++ surface is very close to `entry` but the real handlers
// pick a search-oriented native control: WinUI 3 AutoSuggestBox, GTK4
// GtkSearchEntry, Android SearchView.

#ifndef MPAPP_INTERNAL_BASIC_SEARCH_BAR_HPP
#define MPAPP_INTERNAL_BASIC_SEARCH_BAR_HPP

#include <string>

#include "../command.hpp"
#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class search_bar_handler;

class basic_search_bar : public view {
public:
    basic_search_bar() = default;

    Observable<std::string>   text{};
    Observable<std::string>   placeholder{};

    // Fires when the user submits a search (Enter / IME search action).
    mpapp::signal<const std::string&>  search_button_pressed;

    search_bar_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const search_bar_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                         has_handler() const noexcept { return handler_ != nullptr; }
    void                                         set_handler(search_bar_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    search_bar_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal


#endif // MPAPP_INTERNAL_BASIC_SEARCH_BAR_HPP
