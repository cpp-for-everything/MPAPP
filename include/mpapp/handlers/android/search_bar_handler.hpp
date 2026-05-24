// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_search_bar handler — wraps `android.widget.SearchView`.

#ifndef MPAPP_HANDLERS_ANDROID_SEARCH_BAR_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SEARCH_BAR_HANDLER_HPP

#include "../../platform.hpp"
#include "../../internal/basic_search_bar.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class search_bar_handler<platform::android> {
public:
    search_bar_handler();
    ~search_bar_handler();
    search_bar_handler(const search_bar_handler&)            = delete;
    search_bar_handler& operator=(const search_bar_handler&) = delete;

    void map_text(basic_search_bar& s);
    void map_placeholder(basic_search_bar& s);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_placeholder(const std::string& v);

    struct text_cb_t        { search_bar_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_text(v); } };
    struct placeholder_cb_t { search_bar_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_placeholder(v); } };

    jobject native_ = nullptr;

    text_cb_t                          text_cb_{this};
    placeholder_cb_t                   placeholder_cb_{this};
    signal_slot<const std::string&>    text_slot_{};
    signal_slot<const std::string&>    placeholder_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SEARCH_BAR_HANDLER_HPP
