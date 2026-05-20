// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Per-ADR-0013 data-driven widget dispatch — Android.
//
// Each widget's platform handler `.cpp` self-registers a `view* → jobject`
// function via a static initializer. The container dispatch surfaces
// (stack_layout::add_child, window::child_jobject, scroll_view::child_jobject,
// border::apply_content, content_view::apply_content) call dispatch() first;
// the existing dynamic_cast chains are retained as fallback while the
// migration to self-registration is in progress.

#ifndef MPAPP_HANDLERS_ANDROID_WIDGET_DISPATCH_HPP
#define MPAPP_HANDLERS_ANDROID_WIDGET_DISPATCH_HPP

#if defined(__ANDROID__)

#include <jni.h>

namespace mpapp { class view; }

namespace mpapp::detail::android_dispatch {

using dispatcher_fn = jobject (*)(::mpapp::view*);

void register_dispatcher(dispatcher_fn fn);

jobject dispatch(::mpapp::view* v);

} // namespace mpapp::detail::android_dispatch

#endif // __ANDROID__
#endif // MPAPP_HANDLERS_ANDROID_WIDGET_DISPATCH_HPP
