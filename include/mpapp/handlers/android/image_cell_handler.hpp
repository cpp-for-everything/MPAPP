// SPDX-License-Identifier: Apache-2.0
// Android basic_image_cell handler — horizontal LinearLayout: leading
// ImageView + vertical LinearLayout of two TextViews. Image loaded
// via ImageView.setImageURI(Uri.parse(image_uri)); ImageView handles
// file://, content://, http:// (the latter requires app perms — same
// caveat as MAUI).

#ifndef MPAPP_HANDLERS_ANDROID_IMAGE_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_IMAGE_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_image_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class image_cell_handler<platform::android> {
public:
    image_cell_handler();
    ~image_cell_handler();

    image_cell_handler(const image_cell_handler&)            = delete;
    image_cell_handler& operator=(const image_cell_handler&) = delete;
    image_cell_handler(image_cell_handler&&)                 = delete;
    image_cell_handler& operator=(image_cell_handler&&)      = delete;

    void map_text(basic_image_cell& c);
    void map_detail(basic_image_cell& c);
    void map_image_uri(basic_image_cell& c);

    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_image_cell& /*x*/) noexcept {}


private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);
    void apply_image_uri(const std::string& v);

    struct text_cb_t {
        image_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        image_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };
    struct uri_cb_t {
        image_cell_handler<platform::android>* self;
        void operator()(const std::string& v) const { self->apply_image_uri(v); }
    };

    jobject native_      = nullptr;  // LinearLayout horizontal
    jobject image_view_  = nullptr;  // ImageView
    jobject text_box_    = nullptr;  // LinearLayout vertical
    jobject text_view_   = nullptr;  // TextView (primary)
    jobject detail_view_ = nullptr;  // TextView (detail)

    text_cb_t                       text_cb_{this};
    detail_cb_t                     detail_cb_{this};
    uri_cb_t                        uri_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
    signal_slot<const std::string&> uri_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_IMAGE_CELL_HANDLER_HPP
