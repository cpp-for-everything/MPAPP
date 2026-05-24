// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/TemplatedView.md
//
// `mpapp::templated_view` — single-child container whose surface is
// supplied by an external `ControlTemplate`. Mirrors MAUI's
// `TemplatedView`, the base class for any "lookless" control whose
// visual chrome is theme-driven (search boxes, chips, custom cards).
//
// For M-04b the templating engine is still a P3 design item, so this
// surface is intentionally minimal:
//
//   * `content`     — the live child view rendered by the handler. Real
//                     platforms host this directly via the same
//                     single-child container pattern as `content_view`.
//   * `template_id` — a string handle naming a `ControlTemplate`. The
//                     handlers record it (so XAML/markup wiring can be
//                     verified end-to-end), but the actual template
//                     instantiation step is deferred to the templating
//                     engine ADR. Setting `template_id` does not, in
//                     this revision, replace `content` automatically.
//
// The shape is deliberately analogous to `content_view`: a templated
// view *is* a content view whose contents are supposed to come from a
// template. Until the templating engine lands, the user is expected to
// set `content` themselves (or let an interpreter do so) — what this
// class provides is the typed Observable slot pair.

#ifndef MPAPP_TEMPLATED_VIEW_HPP
#define MPAPP_TEMPLATED_VIEW_HPP

#include <memory>
#include <string>

#include "observable.hpp"
#include "platform.hpp"
#include "view.hpp"

namespace mpapp {

template <class Platform = platform::current>
class templated_view_handler;

class templated_view : public view {
public:
    templated_view()                                     = default;
    ~templated_view() override                           = default;
    templated_view(const templated_view&)                = delete;
    templated_view& operator=(const templated_view&)     = delete;
    templated_view(templated_view&&)                     = delete;
    templated_view& operator=(templated_view&&)          = delete;

    // ----- Properties ----------------------------------------------------
    // The currently-hosted view. When non-null the handler renders it
    // directly into the platform-native single-child container.
    Observable<std::shared_ptr<view>>   content{};

    // String handle for the ControlTemplate that *would* generate
    // `content`. Recorded by each platform handler for end-to-end
    // verification; template instantiation itself is deferred.
    Observable<std::string>             template_id{""};

    // ----- Handler -------------------------------------------------------
    templated_view_handler<platform::current>&       handler() noexcept       { return *handler_; }
    const templated_view_handler<platform::current>& handler() const noexcept { return *handler_; }
    bool                                             has_handler() const noexcept { return handler_ != nullptr; }
    void                                             set_handler(templated_view_handler<platform::current>& h) noexcept { handler_ = &h; }

private:
    templated_view_handler<platform::current>* handler_ = nullptr;
};

} // namespace mpapp

#endif // MPAPP_TEMPLATED_VIEW_HPP
