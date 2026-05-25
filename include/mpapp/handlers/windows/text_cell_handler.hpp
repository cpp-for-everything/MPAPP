// SPDX-License-Identifier: Apache-2.0
// WinUI 3 basic_text_cell handler — vertical StackPanel of two TextBlocks
// (primary text + detail). Wrapping Border gives the cell a consistent
// padding that matches the platform's native row styling.

#ifndef MPAPP_HANDLERS_WINDOWS_TEXT_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_WINDOWS_TEXT_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_text_cell.hpp"

#if defined(_WIN32)

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace mpapp::internal {

template <>
class text_cell_handler<platform::windows> {
public:
    text_cell_handler();
    ~text_cell_handler();

    text_cell_handler(const text_cell_handler&)            = delete;
    text_cell_handler& operator=(const text_cell_handler&) = delete;
    text_cell_handler(text_cell_handler&&)                 = delete;
    text_cell_handler& operator=(text_cell_handler&&)      = delete;

    void map_text(basic_text_cell& c);
    void map_detail(basic_text_cell& c);

    winrt::Microsoft::UI::Xaml::Controls::Border&       native() noexcept       { return native_; }
    const winrt::Microsoft::UI::Xaml::Controls::Border& native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_text_cell& /*x*/) noexcept {}


private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);

    struct text_cb_t {
        text_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        text_cell_handler<platform::windows>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };

    winrt::Microsoft::UI::Xaml::Controls::Border     native_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::StackPanel stack_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock  text_block_{nullptr};
    winrt::Microsoft::UI::Xaml::Controls::TextBlock  detail_block_{nullptr};

    text_cb_t   text_cb_{this};
    detail_cb_t detail_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
};

} // namespace mpapp::internal
#endif // _WIN32
#endif // MPAPP_HANDLERS_WINDOWS_TEXT_CELL_HANDLER_HPP
