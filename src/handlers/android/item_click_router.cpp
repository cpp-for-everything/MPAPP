// SPDX-License-Identifier: Apache-2.0
// Native side of MppItemClickRouter — dispatches AdapterView.onItemClick
// events to list_view / collection_view observables.

#if defined(__ANDROID__)

#include <jni.h>

#include "mpapp/collection_view.hpp"
#include "mpapp/list_view.hpp"

namespace mpapp::detail {

enum class item_click_kind : int {
    list_view       = 0,
    collection_view = 1,
};

void dispatch_android_item_click(jlong owner_ptr, jint kind, jint position) {
    switch (static_cast<item_click_kind>(kind)) {
        case item_click_kind::list_view: {
            auto* lv = reinterpret_cast<list_view*>(owner_ptr);
            if (lv == nullptr) return;
            if (lv->selected_index.get() != static_cast<int>(position)) {
                lv->selected_index.set(static_cast<int>(position));
            }
            lv->item_tapped.emit(static_cast<int>(position));
            break;
        }
        case item_click_kind::collection_view: {
            auto* cv = reinterpret_cast<collection_view*>(owner_ptr);
            if (cv == nullptr) return;
            if (cv->selected_index.get() != static_cast<int>(position)) {
                cv->selected_index.set(static_cast<int>(position));
            }
            cv->item_tapped.emit(static_cast<int>(position));
            break;
        }
    }
}

} // namespace mpapp::detail

extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppItemClickRouter_nativeDispatchItemClick(
    JNIEnv* /*env*/,
    jclass  /*cls*/,
    jlong   owner_ptr,
    jint    kind,
    jint    position) {
    mpapp::detail::dispatch_android_item_click(owner_ptr, kind, position);
}

#endif // __ANDROID__
