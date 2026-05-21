// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. GTK4 frame handler — wraps `GtkBox` as a single-child
// container, applies border color / corner radius / padding through a
// per-handler CSS provider scoped via a unique class name.
//
// `mpapp::frame` is `[[deprecated]]` (MAUI .NET 9 parity); this handler
// IS the legacy path, so it suppresses the diagnostic locally.

#ifndef MPAPP_HANDLERS_LINUX_FRAME_HANDLER_HPP
#define MPAPP_HANDLERS_LINUX_FRAME_HANDLER_HPP

#include <memory>
#include <string>

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif

#include "../../frame.hpp"
#include "../../platform.hpp"
#include "../../signal.hpp"

#if defined(__linux__) && !defined(__ANDROID__)

namespace mpapp {

template <>
class frame_handler<platform::linux_> {
public:
    frame_handler();
    ~frame_handler();

    frame_handler(const frame_handler&)            = delete;
    frame_handler& operator=(const frame_handler&) = delete;
    frame_handler(frame_handler&&)                 = delete;
    frame_handler& operator=(frame_handler&&)      = delete;

    void map_content(frame& f);
    void map_border_color(frame& f);
    void map_has_shadow(frame& f);
    void map_corner_radius(frame& f);
    void map_padding(frame& f);

    void bind_content(frame& f, view& child);

    void*       native() noexcept       { return native_; }
    const void* native() const noexcept { return native_; }

private:
    void apply_content(const std::shared_ptr<view>& v);
    void apply_border_color(const color& c);
    void apply_has_shadow(bool b);
    void apply_corner_radius(float r);
    void apply_padding(const thickness& t);
    void reload_css();

    struct content_cb_t       { frame_handler<platform::linux_>* self; void operator()(const std::shared_ptr<view>& v) const { self->apply_content(v); } };
    struct border_color_cb_t  { frame_handler<platform::linux_>* self; void operator()(const color& c) const { self->apply_border_color(c); } };
    struct has_shadow_cb_t    { frame_handler<platform::linux_>* self; void operator()(bool b) const { self->apply_has_shadow(b); } };
    struct corner_radius_cb_t { frame_handler<platform::linux_>* self; void operator()(float r) const { self->apply_corner_radius(r); } };
    struct padding_cb_t       { frame_handler<platform::linux_>* self; void operator()(const thickness& t) const { self->apply_padding(t); } };

    void* native_   = nullptr;  // GtkBox*
    void* provider_ = nullptr;  // GtkCssProvider*
    std::string class_name_{};
    void* current_child_ = nullptr;  // GtkWidget* — for removal

    color     cached_border_color_{};
    bool      cached_has_shadow_   = true;
    float     cached_corner_radius_ = -1.0f;
    thickness cached_padding_{20.0, 20.0, 20.0, 20.0};

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

} // namespace mpapp

#endif // __linux__ && !__ANDROID__

#if defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif // MPAPP_HANDLERS_LINUX_FRAME_HANDLER_HPP
