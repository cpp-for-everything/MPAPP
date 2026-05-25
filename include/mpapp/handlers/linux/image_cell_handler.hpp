// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_image_cell handler — horizontal GtkBox: leading GtkImage +
// vertical GtkBox of two GtkLabels (text + detail). Image loaded from
// the `image_uri` string via gtk_image_set_from_file (file://, plain
// paths) or gtk_image_set_from_icon_name (for icon: prefix).

#ifndef MPAPP_HANDLERS_LINUX_IMAGE_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_IMAGE_CELL_HANDLER_HPP

#include <string>

#include "../../internal/basic_image_cell.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class image_cell_handler<platform::linux_> {
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

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_image_cell& x);


private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);
    void apply_image_uri(const std::string& v);

    struct text_cb_t {
        image_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        image_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };
    struct uri_cb_t {
        image_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_image_uri(v); }
    };

    void* native_       = nullptr;  // GtkBox* horizontal
    void* image_w_      = nullptr;  // GtkImage*
    void* text_box_     = nullptr;  // GtkBox* vertical (basic_label pair)
    void* text_label_   = nullptr;  // GtkLabel*
    void* detail_label_ = nullptr;  // GtkLabel*

    text_cb_t                       text_cb_{this};
    detail_cb_t                     detail_cb_{this};
    uri_cb_t                        uri_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
    signal_slot<const std::string&> uri_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_IMAGE_CELL_HANDLER_HPP
