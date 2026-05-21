// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/10_Architecture/Components/BindableLayout.md
//
// Android `bindable_layout_handler<platform::android>` — keeps a
// vertical `android.widget.LinearLayout` in sync with the attached-
// property state recorded on the host `layout`. M-04b real surface:
// when `map_items_source(host)` is invoked the LinearLayout is wiped
// via `removeAllViews()` and views resolved through the ADR-0013
// dispatch registry are re-added. `item_template` is recorded but does
// not yet drive instantiation (deferred to the templating ADR — same
// shape as templated_view).
//
// The handler is keyed on the host `layout&` (not on a `bindable_layout`
// instance — `bindable_layout` is a static facility with `= delete`
// constructor).

#ifndef MPAPP_HANDLERS_ANDROID_BINDABLE_LAYOUT_HANDLER_HPP
#define MPAPP_HANDLERS_ANDROID_BINDABLE_LAYOUT_HANDLER_HPP

#include "../../bindable_layout.hpp"
#include "../../layout.hpp"
#include "../../platform.hpp"
#include "../../view.hpp"

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp {

template <>
class bindable_layout_handler<platform::android> {
public:
    bindable_layout_handler();
    ~bindable_layout_handler();

    bindable_layout_handler(const bindable_layout_handler&)            = delete;
    bindable_layout_handler& operator=(const bindable_layout_handler&) = delete;
    bindable_layout_handler(bindable_layout_handler&&)                 = delete;
    bindable_layout_handler& operator=(bindable_layout_handler&&)      = delete;

    void map_items_source(layout& host);
    void map_item_template(layout& host);
    void map_empty_view(layout& host);

    // Global ref to the LinearLayout — owned by the handler and
    // released in the destructor.
    jobject native() noexcept       { return native_; }
    jobject native() const noexcept { return native_; }

private:
    void rebuild_children(layout& host);

    jobject native_ = nullptr;
};

} // namespace mpapp

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_BINDABLE_LAYOUT_HANDLER_HPP
