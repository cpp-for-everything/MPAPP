// SPDX-License-Identifier: Apache-2.0
// `mpapp::internal::basic_popup` — CommunityToolkit Popup surface.
// Single content slot, open/dismiss observables, and opened/closed signals.
// Mirrors the .NET MAUI CommunityToolkit `Popup` control mock-first (ADR-0008).

#ifndef MPAPP_INTERNAL_BASIC_POPUP_HPP
#define MPAPP_INTERNAL_BASIC_POPUP_HPP

#include <memory>
#include <optional>
#include <string>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp::internal {

template <class Platform = platform::current>
class popup_handler;

class basic_popup : public view {
public:
    basic_popup() = default;

    // ----- Content slot ---------------------------------------------------
    // The single child view rendered inside the popup. Null = no content.
    Observable<std::shared_ptr<view>> content{};

    // ----- State properties -----------------------------------------------
    // Whether the popup is currently open / visible.
    Observable<bool> is_open{false};

    // Whether the user can dismiss the popup by tapping the area outside it.
    Observable<bool> can_be_dismissed_by_tapping_outside{true};

    // ----- Close with result ----------------------------------------------
    // Call close() to dismiss the popup and store the result string.
    // The closed signal is emitted with the result (nullopt when dismissed
    // without a result, e.g. by tapping outside).
    void close(std::optional<std::string> result = std::nullopt) {
        result_ = result;
        is_open = false;
        closed.emit(result_);
    }

    // [[nodiscard]] getter — lets test code read back the stored result.
    [[nodiscard]] const std::optional<std::string>& result() const noexcept {
        return result_;
    }

    // ----- Signals --------------------------------------------------------
    mpapp::signal<>                                opened;
    mpapp::signal<const std::optional<std::string>&> closed;

    // ----- Handler attachment (pointer-based, opt-in) --------------------
    popup_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const popup_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                    has_handler() const noexcept { return handler_ != nullptr; }
    void                                    set_handler(popup_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    std::optional<std::string>       result_{};
    popup_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_POPUP_HPP
