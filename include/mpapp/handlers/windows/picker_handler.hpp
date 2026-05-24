// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. WinUI 3 basic_picker handler — wraps `mux::Controls::ComboBox`.

#ifndef MPAPP_HANDLERS_WINDOWS_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_PICKER_HANDLER_HPP

#include <string>
#include <vector>

#include "../../internal/basic_picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class picker_handler<platform::windows> {
public:
    picker_handler();
    ~picker_handler();
    picker_handler(const picker_handler&)            = delete;
    picker_handler& operator=(const picker_handler&) = delete;

    void map_items(basic_picker& p);
    void map_selected_index(basic_picker& p);
    void map_title(basic_picker& p);

    winrt::Microsoft::UI::Xaml::Controls::ComboBox&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::ComboBox& native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<std::string>& v);
    void apply_selected_index(int v);
    void apply_title(const std::string& v);

    struct items_cb_t    { picker_handler<platform::windows>* self; void operator()(const std::vector<std::string>& v) const { self->apply_items(v); } };
    struct selected_cb_t { picker_handler<platform::windows>* self; void operator()(int v) const { self->apply_selected_index(v); } };
    struct title_cb_t    { picker_handler<platform::windows>* self; void operator()(const std::string& v) const { self->apply_title(v); } };

    winrt::Microsoft::UI::Xaml::Controls::ComboBox native_{nullptr};
    bool suppress_echo_ = false;

    items_cb_t                                          items_cb_{this};
    selected_cb_t                                       selected_cb_{this};
    title_cb_t                                          title_cb_{this};
    signal_slot<std::vector<std::string> const&>        items_slot_{};
    signal_slot<const int&>                             selected_slot_{};
    signal_slot<const std::string&>                     title_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_PICKER_HANDLER_HPP
