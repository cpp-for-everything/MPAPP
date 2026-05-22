// SPDX-License-Identifier: Apache-2.0
// Android collection_view handler implementation.

#include "mpapp/handlers/android/collection_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

constexpr jint ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1 = 0x01090003;
constexpr jint CHOICE_MODE_SINGLE                  = 1;
constexpr jint CHOICE_MODE_MULTIPLE                = 2;
constexpr jint CHOICE_MODE_NONE                    = 0;

// MATCH_PARENT for FrameLayout.LayoutParams.
constexpr jint LP_MATCH_PARENT = -1;

jobject make_object(JNIEnv* env, const char* cls_name, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass(cls_name);
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, context);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(cls);
        return nullptr;
    }
    env->DeleteLocalRef(cls);
    jobject global = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return global;
}

void framelayout_add_match(JNIEnv* env, jobject parent, jobject child) {
    if (child == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass lp_cls = env->FindClass("android/widget/FrameLayout$LayoutParams");
    if (lp_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(lp_cls, "<init>", "(II)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(lp_cls); return; }
    jobject lp = env->NewObject(lp_cls, ctor, LP_MATCH_PARENT, LP_MATCH_PARENT);
    env->DeleteLocalRef(lp_cls);
    if (lp == nullptr) { env->ExceptionClear(); return; }

    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls != nullptr) {
        jmethodID set_lp = env->GetMethodID(view_cls, "setLayoutParams",
            "(Landroid/view/ViewGroup$LayoutParams;)V");
        if (set_lp != nullptr) {
            env->CallVoidMethod(child, set_lp, lp);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(view_cls);
    }
    env->DeleteLocalRef(lp);

    jclass vg = env->FindClass("android/view/ViewGroup");
    if (vg != nullptr) {
        jmethodID add = env->GetMethodID(vg, "addView", "(Landroid/view/View;)V");
        if (add != nullptr) {
            env->CallVoidMethod(parent, add, child);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(vg);
    }
}

void vg_remove_all(JNIEnv* env, jobject parent) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "removeAllViews", "()V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void install_adapter(JNIEnv* env, jobject context, jobject adapter_view,
                     const std::vector<std::string>& items) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass string_cls = env->FindClass("java/lang/String");
    if (string_cls == nullptr) { env->ExceptionClear(); return; }
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(items.size()), string_cls, nullptr);
    if (arr == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(string_cls);
        return;
    }
    for (jsize i = 0; i < static_cast<jsize>(items.size()); ++i) {
        jstring s = env->NewStringUTF(items[static_cast<std::size_t>(i)].c_str());
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    env->DeleteLocalRef(string_cls);

    jclass adapter_cls = env->FindClass("android/widget/ArrayAdapter");
    if (adapter_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(arr); return; }
    jmethodID ctor = env->GetMethodID(adapter_cls, "<init>",
        "(Landroid/content/Context;I[Ljava/lang/Object;)V");
    if (ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(adapter_cls);
        env->DeleteLocalRef(arr);
        return;
    }
    jobject adapter = env->NewObject(adapter_cls, ctor, context,
                                     ANDROID_R_LAYOUT_SIMPLE_LIST_ITEM_1,
                                     arr);
    env->DeleteLocalRef(arr);
    if (env->ExceptionCheck() || adapter == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(adapter_cls);
        return;
    }
    env->DeleteLocalRef(adapter_cls);

    // setAdapter lives on AdapterView<T> — shared base of ListView and
    // GridView. The expected argument type differs across the two
    // (ListAdapter for ListView, ListAdapter for GridView in practice
    // since ArrayAdapter implements both), so the generic descriptor
    // works.
    jclass av_cls = env->FindClass("android/widget/AdapterView");
    if (av_cls != nullptr) {
        // For ListView: setAdapter(ListAdapter); for GridView: same.
        // We use the concrete subclass to dispatch via the right vtable.
        jclass concrete = env->GetObjectClass(adapter_view);
        jmethodID set_adapter = env->GetMethodID(concrete, "setAdapter",
            "(Landroid/widget/ListAdapter;)V");
        if (set_adapter != nullptr) {
            env->CallVoidMethod(adapter_view, set_adapter, adapter);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(concrete);
        env->DeleteLocalRef(av_cls);
    }
    env->DeleteLocalRef(adapter);
}

void adapter_view_set_selection(JNIEnv* env, jobject av, int idx) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->GetObjectClass(av);
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setSelection", "(I)V");
    if (m != nullptr && idx >= 0) {
        env->CallVoidMethod(av, m, static_cast<jint>(idx));
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void abs_list_view_set_choice_mode(JNIEnv* env, jobject av, jint mode) {
    // setChoiceMode is on android.widget.AbsListView (parent of both
    // ListView and GridView).
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/AbsListView");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setChoiceMode", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(av, m, mode);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void grid_view_set_num_columns_auto(JNIEnv* env, jobject gv) {
    // GridView.setNumColumns(GridView.AUTO_FIT) lets the system pick a
    // sensible default column count for the screen width.
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass gv_cls = env->FindClass("android/widget/GridView");
    if (gv_cls == nullptr) { env->ExceptionClear(); return; }
    constexpr jint AUTO_FIT = -1;
    jmethodID m = env->GetMethodID(gv_cls, "setNumColumns", "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(gv, m, AUTO_FIT);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(gv_cls);
}

void install_item_click_router(JNIEnv* env, jobject adapter_view, collection_view* cv) {
    if (env == nullptr || adapter_view == nullptr || cv == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass router_cls = env->FindClass("io/mpapp/MppItemClickRouter");
    if (router_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID ctor = env->GetMethodID(router_cls, "<init>", "(JI)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(router_cls); return; }
    jobject router = env->NewObject(router_cls, ctor,
                                    reinterpret_cast<jlong>(cv),
                                    static_cast<jint>(1 /* collection_view kind */));
    if (env->ExceptionCheck() || router == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(router_cls);
        return;
    }
    env->DeleteLocalRef(router_cls);

    jclass av_cls = env->FindClass("android/widget/AdapterView");
    if (av_cls != nullptr) {
        jmethodID set_listener = env->GetMethodID(av_cls, "setOnItemClickListener",
            "(Landroid/widget/AdapterView$OnItemClickListener;)V");
        if (set_listener != nullptr) {
            env->CallVoidMethod(adapter_view, set_listener, router);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(av_cls);
    }
    env->DeleteLocalRef(router);
}

} // namespace

collection_view_handler<platform::android>::collection_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_object(env, "android/widget/FrameLayout", detail::get_activity());
    rebuild_inner_for_layout(collection_layout::vertical_list);
}

collection_view_handler<platform::android>::~collection_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (inner_  != nullptr) { env->DeleteGlobalRef(inner_);  inner_  = nullptr; }
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void collection_view_handler<platform::android>::rebuild_inner_for_layout(collection_layout l) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    if (inner_ != nullptr) {
        vg_remove_all(env, native_);
        env->DeleteGlobalRef(inner_);
        inner_ = nullptr;
    }

    const bool want_grid = (l == collection_layout::vertical_grid
                         || l == collection_layout::horizontal_grid);
    inner_ = make_object(env,
        want_grid ? "android/widget/GridView" : "android/widget/ListView",
        detail::get_activity());
    is_grid_ = want_grid;

    if (inner_ == nullptr) return;
    if (want_grid) grid_view_set_num_columns_auto(env, inner_);
    abs_list_view_set_choice_mode(env, inner_, CHOICE_MODE_SINGLE);
    framelayout_add_match(env, native_, inner_);
}

void collection_view_handler<platform::android>::rebuild_items(const std::vector<std::string>& v) {
    if (inner_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    install_adapter(env, detail::get_activity(), inner_, v);
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

namespace {

// In typed mode the outer FrameLayout wraps a ScrollView containing a
// vertical LinearLayout. Each typed item's native View is added as a
// direct child. Selection / multi-select are not supported in this
// mode — typed cells own their interaction surface (taps, toggles,
// IME completion all flow through each cell's own native event path).

void linear_set_orientation_vertical(JNIEnv* env, jobject ll) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/LinearLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "setOrientation", "(I)V");
    constexpr jint VERTICAL = 1;
    if (m != nullptr) {
        env->CallVoidMethod(ll, m, VERTICAL);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

void vg_add(JNIEnv* env, jobject parent, jobject child) {
    if (child == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView", "(Landroid/view/View;)V");
    if (m != nullptr) {
        env->CallVoidMethod(parent, m, child);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

void collection_view_handler<platform::android>::rebuild_typed(const std::vector<view*>& items) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    // Tear down the current AdapterView inner (if any).
    if (inner_ != nullptr) {
        vg_remove_all(env, native_);
        env->DeleteGlobalRef(inner_);
        inner_ = nullptr;
    }

    // Replace with ScrollView + LinearLayout(VERTICAL).
    jobject scroll = make_object(env, "android/widget/ScrollView", detail::get_activity());
    jobject lin    = make_object(env, "android/widget/LinearLayout", detail::get_activity());
    if (lin != nullptr) linear_set_orientation_vertical(env, lin);
    if (scroll != nullptr && lin != nullptr) {
        // ScrollView extends FrameLayout — addView with MATCH_PARENT
        // layout params lets the inner LinearLayout fill it.
        framelayout_add_match(env, scroll, lin);
    }
    if (native_ != nullptr && scroll != nullptr) {
        framelayout_add_match(env, native_, scroll);
    }

    // Populate the LinearLayout with each cell/view's native handle.
    for (view* item : items) {
        if (item == nullptr || lin == nullptr) continue;
        jobject native_item = detail::android_dispatch::dispatch(item);
        if (native_item != nullptr) {
            vg_add(env, lin, native_item);
        }
    }

    // Keep the ScrollView as inner_ so destructor releases it. The
    // LinearLayout is a local ref — we let ScrollView retain it as a
    // child after addView; once we release `scroll` as a local, we
    // re-promote to global.
    if (lin != nullptr) env->DeleteLocalRef(lin);
    if (scroll != nullptr) {
        inner_ = env->NewGlobalRef(scroll);
        env->DeleteLocalRef(scroll);
    }
    is_grid_ = false;
}

void collection_view_handler<platform::android>::rebuild_active() {
    if (bound_ == nullptr) return;
    if (!bound_->typed_items.get().empty()) {
        rebuild_typed(bound_->typed_items.get());
    } else {
        // Flat mode — ensure inner_ matches the current layout enum.
        const collection_layout l = bound_->layout.get();
        const bool want_grid = (l == collection_layout::vertical_grid
                             || l == collection_layout::horizontal_grid);
        // If inner_ doesn't exist or is wrong type, rebuild.
        rebuild_inner_for_layout(l);
        (void)want_grid;
        apply_selection_mode(bound_->selection_mode.get());
        rebuild_items(bound_->items_source.get());
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) install_item_click_router(env, inner_, bound_);
    }
}

void collection_view_handler<platform::android>::apply_selection(int idx) {
    if (inner_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    adapter_view_set_selection(env, inner_, idx);
}

void collection_view_handler<platform::android>::apply_selection_mode(collection_selection_mode m) {
    if (inner_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jint mode = CHOICE_MODE_SINGLE;
    switch (m) {
        case collection_selection_mode::none:     mode = CHOICE_MODE_NONE;     break;
        case collection_selection_mode::multiple: mode = CHOICE_MODE_MULTIPLE; break;
        case collection_selection_mode::single:
        default:                                  mode = CHOICE_MODE_SINGLE;   break;
    }
    abs_list_view_set_choice_mode(env, inner_, mode);
}

void collection_view_handler<platform::android>::apply_layout(collection_layout l) {
    if (bound_ != nullptr && !bound_->typed_items.get().empty()) {
        // In typed mode the layout enum is ignored — we use a
        // vertical LinearLayout regardless. Skip the inner rebuild.
        return;
    }
    const bool want_grid = (l == collection_layout::vertical_grid
                         || l == collection_layout::horizontal_grid);
    if (want_grid == is_grid_ && inner_ != nullptr) return;

    rebuild_inner_for_layout(l);
    if (bound_ != nullptr) {
        apply_selection_mode(bound_->selection_mode.get());
        rebuild_items(bound_->items_source.get());
        JNIEnv* env = detail::attach_current_thread();
        if (env != nullptr) install_item_click_router(env, inner_, bound_);
    }
}

void collection_view_handler<platform::android>::refresh_multi_selection_from_native() {
    if (inner_ == nullptr || bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();

    // SparseBooleanArray AbsListView.getCheckedItemPositions()
    jclass av_cls = env->FindClass("android/widget/AbsListView");
    if (av_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID get_checked = env->GetMethodID(av_cls, "getCheckedItemPositions",
        "()Landroid/util/SparseBooleanArray;");
    jobject sparse = (get_checked != nullptr) ? env->CallObjectMethod(inner_, get_checked) : nullptr;
    env->DeleteLocalRef(av_cls);
    if (sparse == nullptr) { env->ExceptionClear(); return; }

    jclass sba_cls = env->FindClass("android/util/SparseBooleanArray");
    if (sba_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(sparse);
        return;
    }
    jmethodID size_m = env->GetMethodID(sba_cls, "size",    "()I");
    jmethodID key_at = env->GetMethodID(sba_cls, "keyAt",   "(I)I");
    jmethodID val_at = env->GetMethodID(sba_cls, "valueAt", "(I)Z");
    std::vector<int> idxs;
    if (size_m != nullptr && key_at != nullptr && val_at != nullptr) {
        const jint n = env->CallIntMethod(sparse, size_m);
        idxs.reserve(static_cast<std::size_t>(n));
        for (jint i = 0; i < n; ++i) {
            jboolean v = env->CallBooleanMethod(sparse, val_at, i);
            if (v == JNI_TRUE) {
                jint k = env->CallIntMethod(sparse, key_at, i);
                idxs.push_back(static_cast<int>(k));
            }
        }
    }
    env->DeleteLocalRef(sba_cls);
    env->DeleteLocalRef(sparse);
    if (env->ExceptionCheck()) env->ExceptionClear();

    if (bound_->selected_indices.get() != idxs) {
        bound_->selected_indices.set(std::move(idxs));
    }
}

void collection_view_handler<platform::android>::map_items_source(collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::android>::map_typed_items(collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.typed_items.changed.subscribe(typed_slot_, typed_cb_);
}

void collection_view_handler<platform::android>::map_selected_index(collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::android>::map_selection_mode(collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

void collection_view_handler<platform::android>::map_layout(collection_view& cv) {
    apply_layout(cv.layout.get());
    cv.layout.changed.subscribe(layout_slot_, layout_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::collection_view*>(v); c && c->has_cv_handler()) {
        return c->cv_handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_collection_view); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
