// SPDX-License-Identifier: Apache-2.0
// GTK4 basic_text_cell handler — vertical GtkBox of two GtkLabels (text +
// detail). Both labels start-aligned; detail basic_label hidden when empty.

#ifndef MPAPP_HANDLERS_LINUX_TEXT_CELL_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_TEXT_CELL_HANDLER_HPP

#include <string>

#include "../../platform.hpp"
#include "../../signal.hpp"
#include "../../internal/basic_text_cell.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp::internal {

template <>
class text_cell_handler<platform::linux_> {
public:
    text_cell_handler();
    ~text_cell_handler();

    text_cell_handler(const text_cell_handler&)            = delete;
    text_cell_handler& operator=(const text_cell_handler&) = delete;
    text_cell_handler(text_cell_handler&&)                 = delete;
    text_cell_handler& operator=(text_cell_handler&&)      = delete;

    void map_text(basic_text_cell& c);
    void map_detail(basic_text_cell& c);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

// RFC-0003: walks `x.gesture_recognizers` and installs

// matching GtkGesture* controllers via

// `mpapp::internal::linux_gestures::attach`.

void map_gestures(basic_text_cell& x);


private:
    void apply_text(const std::string& v);
    void apply_detail(const std::string& v);

    struct text_cb_t {
        text_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_text(v); }
    };
    struct detail_cb_t {
        text_cell_handler<platform::linux_>* self;
        void operator()(const std::string& v) const { self->apply_detail(v); }
    };

    void* native_       = nullptr;  // GtkBox* (vertical)
    void* text_label_   = nullptr;  // GtkLabel*
    void* detail_label_ = nullptr;  // GtkLabel*

    text_cb_t   text_cb_{this};
    detail_cb_t detail_cb_{this};
    signal_slot<const std::string&> text_slot_{};
    signal_slot<const std::string&> detail_slot_{};
};

} // namespace mpapp::internal
#endif // __linux__ && !__ANDROID__
#endif // MPAPP_HANDLERS_LINUX_TEXT_CELL_HANDLER_HPP
