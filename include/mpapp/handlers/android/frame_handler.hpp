// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Android basic_frame handler — wraps `android.widget.FrameLayout`
// with a `GradientDrawable` background carrying the basic_border color and
// corner radius.
//
// `mpapp::basic_frame` is `[[deprecated]]` (MAUI .NET 9 parity); this handler
// IS the legacy path, so it suppresses the diagnostic locally.

#ifndef MPAPP_HANDLERS_ANDROID_FRAME_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_FRAME_HANDLER_HPP

#include <memory>

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include "../../internal/basic_frame.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp::internal {

template <>
class frame_handler<platform::android> {
public:
    frame_handler();
    ~frame_handler();

    frame_handler(const frame_handler&)            = delete;
    frame_handler& operator=(const frame_handler&) = delete;
    frame_handler(frame_handler&&)                 = delete;
    frame_handler& operator=(frame_handler&&)      = delete;

    void map_content(basic_frame& f);
    void map_border_color(basic_frame& f);
    void map_has_shadow(basic_frame& f);
    void map_corner_radius(basic_frame& f);
    void map_padding(basic_frame& f);

    void bind_content(basic_frame& f, view& child);

    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

// RFC-0003 stub: per-platform real gesture wire-up is

// pending the platform's real-handler task. No-op today

// so the wrapper ctor's unconditional

// `embedded_handler_.map_gestures(*this);` links.

void map_gestures(basic_frame& /*x*/) noexcept {}


private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_border_color(const color& c);
    void apply_has_shadow(bool b);
    void apply_corner_radius(float r);
    void apply_padding(const thickness& t);
    void rebuild_background();

    struct content_cb_t       { frame_handler<platform::android>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct border_color_cb_t  { frame_handler<platform::android>* self; void operator()(const color& c) const { self->apply_border_color(c); } };
    struct has_shadow_cb_t    { frame_handler<platform::android>* self; void operator()(bool b) const { self->apply_has_shadow(b); } };
    struct corner_radius_cb_t { frame_handler<platform::android>* self; void operator()(float r) const { self->apply_corner_radius(r); } };
    struct padding_cb_t       { frame_handler<platform::android>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };

    jobject native_ = nullptr;  // FrameLayout global ref

    color  cached_border_color_{};
    bool   cached_has_shadow_   = true;
    float  cached_corner_radius_ = -1.0f;

    content_cb_t                              content_cb_{this};
    border_color_cb_t                         border_color_cb_{this};
    has_shadow_cb_t                           has_shadow_cb_{this};
    corner_radius_cb_t                        corner_radius_cb_{this};
    padding_cb_t                              padding_cb_{this};
    signal_slot<std::shared_ptr<view> const&> content_slot_{};
    signal_slot<const color&>                 border_color_slot_{};
    signal_slot<const bool&>                  has_shadow_slot_{};
    signal_slot<const float&>                 corner_radius_slot_{};
    signal_slot<const thickness&>             padding_slot_{};
};

} // namespace mpapp::internal
#endif // __ANDROID__

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // MPAPP_HANDLERS_ANDROID_FRAME_HANDLER_HPP
