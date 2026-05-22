// SPDX-License-Identifier: Apache-2.0
// Android shape_view handler — wraps a custom `MppShapeView` (extends
// android.view.View) whose onDraw renders the configured shape kind
// against the Canvas with the configured paint properties. The native
// handler pushes property updates through JNI setters and the Java
// side re-invalidates on each change.

#ifndef MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP

#include <cstdint>
#include <string>

#include "../../platform.hpp"
#include "../../shape_view.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class shape_view_handler<platform::android> {
public:
    shape_view_handler();
    ~shape_view_handler();

    shape_view_handler(const shape_view_handler&)            = delete;
    shape_view_handler& operator=(const shape_view_handler&) = delete;
    shape_view_handler(shape_view_handler&&)                 = delete;
    shape_view_handler& operator=(shape_view_handler&&)      = delete;

    void map_kind(shape_view& s);
    void map_data(shape_view& s);
    void map_fill(shape_view& s);
    void map_stroke(shape_view& s);
    void map_stroke_thickness(shape_view& s);
    void map_opacity(shape_view& s);

    jobject native() const noexcept { return native_; }

private:
    void apply_kind(shape_kind v);
    void apply_data(const std::string& v);
    void apply_fill(const std::string& v);
    void apply_stroke(const std::string& v);
    void apply_stroke_thickness(double v);
    void apply_opacity(double v);

    struct kind_cb_t {
        shape_view_handler<platform::android>* self;
        void operator()(shape_kind v) const { self->apply_kind(v); }
    };
    struct str_cb_t {
        shape_view_handler<platform::android>* self;
        enum which_t { fill_, stroke_, data_ } which;
        void operator()(const std::string& v) const {
            if (which == fill_)   self->apply_fill(v);
            else if (which == stroke_) self->apply_stroke(v);
            else self->apply_data(v);
        }
    };
    struct dbl_cb_t {
        shape_view_handler<platform::android>* self;
        enum which_t { thickness_, opacity_ } which;
        void operator()(double v) const {
            if (which == thickness_) self->apply_stroke_thickness(v);
            else self->apply_opacity(v);
        }
    };

    jobject     native_ = nullptr;  // MppShapeView (global ref)
    shape_view* bound_  = nullptr;

    kind_cb_t                       kind_cb_{this};
    str_cb_t                        data_cb_{this, str_cb_t::data_};
    str_cb_t                        fill_cb_{this, str_cb_t::fill_};
    str_cb_t                        stroke_cb_{this, str_cb_t::stroke_};
    dbl_cb_t                        stroke_thick_cb_{this, dbl_cb_t::thickness_};
    dbl_cb_t                        opacity_cb_{this, dbl_cb_t::opacity_};
    signal_slot<const shape_kind&>  kind_slot_{};
    signal_slot<const std::string&> data_slot_{};
    signal_slot<const std::string&> fill_slot_{};
    signal_slot<const std::string&> stroke_slot_{};
    signal_slot<const double&>      stroke_thick_slot_{};
    signal_slot<const double&>      opacity_slot_{};
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_SHAPE_VIEW_HANDLER_HPP
