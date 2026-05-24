// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_search_bar handler — wraps
// `mux::Controls::AutoSuggestBox`. Provides search-affordance styling
// without the suggestions popup (we don't expose IsSuggestionListOpen
// on the cross-platform surface yet).

#ifndef MPAPP_HANDLERS_WINDOWS_SEARCH_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_SEARCH_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_search_bar.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class search_bar_handler<platform::windows> {
public:
    search_bar_handler();
    ~search_bar_handler();
    search_bar_handler(const search_bar_handler&)            = delete;
    search_bar_handler& operator=(const search_bar_handler&) = delete;

    void map_text(basic_search_bar& s);
    void map_placeholder(basic_search_bar& s);

    winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox& native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);

    struct text_cb_t        { search_bar_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct placeholder_cb_t { search_bar_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_placeholder(v); } };

    winrt::Microsoft::UI::Xaml::Controls::AutoSuggestBox native_{nullptr};
    bool suppress_echo_ = false;

    text_cb_t                          text_cb_{this};
    placeholder_cb_t                   placeholder_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const std::string&>    placeholder_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_SEARCH_BAR_HANDLER_HPP
