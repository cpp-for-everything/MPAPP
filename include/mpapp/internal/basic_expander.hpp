// SPDX-License-Identifier: Apache-2.0
// `mpapp::internal::basic_expander` — collapsible container with a header
// slot and a content slot. Mirrors the CommunityToolkit.Maui `Expander`
// control surface: the `is_expanded` Observable drives the `expanded` /
// `collapsed` signals whenever the bool value actually changes.

#ifndef MPAPP_INTERNAL_BASIC_EXPANDER_HPP
#define MPAPP_INTERNAL_BASIC_EXPANDER_HPP

#include <cstdint>
#if __has_include(<format>) && !defined(__ANDROID__)
#  include <format>
#  define MPAPP_EXPANDER_HAS_STD_FORMAT 1
#endif
#include <string_view>

#include "../observable.hpp"
#include "../platform.hpp"
#include "../signal.hpp"
#include "../view.hpp"

namespace mpapp {

// Direction in which the content panel opens relative to the header.
enum class expand_direction : std::uint8_t {
    down = 0,
    up   = 1,
};

constexpr std::string_view to_string(expand_direction d) noexcept {
    switch (d) {
        case expand_direction::down: return "down";
        case expand_direction::up:   return "up";
    }
    return "?";
}

} // namespace mpapp

#ifdef MPAPP_EXPANDER_HAS_STD_FORMAT

template <>
struct std::formatter<mpapp::expand_direction> : std::formatter<std::string_view> {
    auto format(mpapp::expand_direction d, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(mpapp::to_string(d), ctx);
    }
};

#endif // MPAPP_EXPANDER_HAS_STD_FORMAT

namespace mpapp::internal {

// Forward-declared primary template. Concrete specialisations live in
// `mpapp/handlers/<platform>/expander_handler.hpp` and
// `mpapp/handlers/mock/expander_handler.hpp`.
template <class Platform = platform::current>
class expander_handler;

class basic_expander : public view {
public:
    basic_expander() {
        // Wire is_expanded changes to fire the appropriate signal.
        is_expanded.changed.subscribe(expanded_observer_slot_, expanded_observer_cb_);
    }

    basic_expander(const basic_expander&)            = delete;
    basic_expander& operator=(const basic_expander&) = delete;
    basic_expander(basic_expander&&)                 = delete;
    basic_expander& operator=(basic_expander&&)      = delete;

    // ----- Slots -----------------------------------------------------------
    // Header view — always visible; user taps/clicks it to toggle expansion.
    [[nodiscard]] view* header() const noexcept { return header_; }
    void                set_header(view* h) noexcept { header_ = h; }

    // Content view — shown when is_expanded == true, hidden otherwise.
    [[nodiscard]] view* content() const noexcept { return content_; }
    void                set_content(view* c) noexcept { content_ = c; }

    // ----- Properties ------------------------------------------------------
    Observable<bool>             is_expanded{false};
    Observable<expand_direction> direction{expand_direction::down};

    // ----- Signals ---------------------------------------------------------
    // Fired when is_expanded transitions false -> true.
    mpapp::signal<> expanded;
    // Fired when is_expanded transitions true -> false.
    mpapp::signal<> collapsed;

    // ----- Handler attachment (pointer-based, opt-in) ----------------------
    [[nodiscard]] expander_handler<platform::current>&       handler() noexcept       { return *handler_; }
    [[nodiscard]] const expander_handler<platform::current>& handler() const noexcept { return *handler_; }
    [[nodiscard]] bool                                        has_handler() const noexcept { return handler_ != nullptr; }
    void                                                      set_handler(expander_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    // Watches is_expanded and routes to expanded / collapsed.
    struct expanded_observer_cb_t {
        basic_expander* self;
        void operator()(const bool& value) const {
            if (value) {
                self->expanded.emit();
            } else {
                self->collapsed.emit();
            }
        }
    };

    view*                    header_   = nullptr;
    view*                    content_  = nullptr;
    expander_handler<platform::current>* handler_ = nullptr;

    expanded_observer_cb_t      expanded_observer_cb_{this};
    signal_slot<const bool&>    expanded_observer_slot_{};
};

} // namespace mpapp::internal

#endif // MPAPP_INTERNAL_BASIC_EXPANDER_HPP
