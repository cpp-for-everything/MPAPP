// SPDX-License-Identifier: Apache-2.0
// Android basic_text_cell handler — vertical LinearLayout of two TextViews.

#ifndef MPAPP_HANDLERS_ANDROID_TEXT_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_TEXT_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_text_cell.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class text_cell_handler<platform::android> {
public:
    text_cell_handler();
    ~text_cell_handler();

    text_cell_handler(const text_cell_handler&)            = delete;
    text_cell_handler& operator=(const text_cell_handler&) = delete;
    text_cell_handler(text_cell_handler&&)                 = delete;
    text_cell_handler& operator=(text_cell_handler&&)      = delete;

    void map_text(basic_text_cell& c);
    void map_detail(basic_text_cell& c);

    jobject native() const noexcept { return native_; }

private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);

    struct text_cb_t {
        text_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        text_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };

    jobject native_       = nullptr;  // LinearLayout vertical
    jobject text_view_    = nullptr;  // TextView (primary)
    jobject detail_view_  = nullptr;  // TextView (detail)

    text_cb_t   text_cb_{this};
    detail_cb_t detail_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_TEXT_CELL_HANDLER_HPP
