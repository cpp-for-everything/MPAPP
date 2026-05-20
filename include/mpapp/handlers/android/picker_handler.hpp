// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android picker handler — wraps `android.widget.Spinner`
// + `ArrayAdapter<String>`.

#ifndef MPAPP_HANDLERS_ANDROID_PICKER_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_PICKER_HANDLER_HPP

#include <string>
#include <vector>

#include "../../picker.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class picker_handler<platform::android> {
public:
    picker_handler();
    ~picker_handler();
    picker_handler(const picker_handler&)            = delete;
    picker_handler& operator=(const picker_handler&) = delete;

    void map_items(picker& p);
    void map_selected_index(picker& p);
    void map_title(picker& p);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void apply_items(const std::vector<std::string>& v);
    void apply_selected_index(int v);
    void apply_title(const std::string& /*v*/) { /* Spinner has no first-class title; deferred. */ }

    struct items_cb_t    { picker_handler<platform::android>* self; void operator()(const std::vector<std::string>& v) const { self->apply_items(v); } };
    struct selected_cb_t { picker_handler<platform::android>* self; void operator()(int v) const { self->apply_selected_index(v); } };
    struct title_cb_t    { picker_handler<platform::android>* self; void operator()(const std::string& v) const { self->apply_title(v); } };

    jobject native_ = nullptr;  // Spinner global ref
    bool    suppress_echo_ = false;

    items_cb_t                                          items_cb_{this};
    selected_cb_t                                       selected_cb_{this};
    title_cb_t                                          title_cb_{this};
    signal_slot<std::vector<std::string> const&>        items_slot_{};
    signal_slot<const int&>                             selected_slot_{};
    signal_slot<const std::string&>                     title_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_PICKER_HANDLER_HPP
