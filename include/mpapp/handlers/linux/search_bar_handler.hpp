// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 basic_search_bar handler — wraps `GtkSearchEntry`.

#ifndef MPAPP_HANDLERS_LINUX_SEARCH_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_SEARCH_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_search_bar.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class search_bar_handler<platform::linux_> {
public:
    search_bar_handler();
    ~search_bar_handler();
    search_bar_handler(const search_bar_handler&)            = delete;
    search_bar_handler& operator=(const search_bar_handler&) = delete;

    void map_text(basic_search_bar& s);
    void map_placeholder(basic_search_bar& s);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_search_bar& x);


private:
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);

    struct text_cb_t        { search_bar_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct placeholder_cb_t { search_bar_handler<platform::linux_>* self; void operator()(const std::string& v) const { self->apply_placeholder(v); } };

    void* native_ = nullptr;
    bool  suppress_echo_ = false;

    text_cb_t                          text_cb_{this};
    placeholder_cb_t                   placeholder_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const std::string&>    placeholder_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_SEARCH_BAR_HANDLER_HPP
