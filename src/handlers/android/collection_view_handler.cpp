// SPDX-License-Identifier: Apache-2.0
// Android basic_collection_view handler implementation. T-0028: migrated
// from android.widget.ListView/GridView (vertical-only) to
// androidx.recyclerview.widget.RecyclerView with a swappable
// LayoutManager — covers all four collection_layout values:
//
//   vertical_list   → LinearLayoutManager(VERTICAL)
//   horizontal_list → LinearLayoutManager(HORIZONTAL)
//   vertical_grid   → GridLayoutManager(span, VERTICAL)
//   horizontal_grid → GridLayoutManager(span, HORIZONTAL)
//
// The MppCollectionAdapter (see examples/android_hello/.../io/mpapp/
// MppCollectionAdapter.java) handles both string and native-view
// payloads and pushes selection state back to native via
// MppItemClickRouter's package-private nativeDispatchCheckedSet hook.

#include "mpapp/handlers/android/collection_view_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp::internal {

namespace {

constexpr jint LP_MATCH_PARENT = -1;
constexpr jint RV_VERTICAL     = 1;   // RecyclerView.VERTICAL
constexpr jint RV_HORIZONTAL   = 0;   // RecyclerView.HORIZONTAL

// Span fallback for GridLayoutManager when the surface has the default
// span=1: RecyclerView has no AUTO_FIT equivalent, so we pick a
// sensible default of 2 columns for grid layouts to match the
// "multi-column by default" behavior the original GridView gave.
constexpr jint DEFAULT_GRID_SPAN = 2;

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

// --- RecyclerView / LayoutManager / Adapter helpers ---------------------

jobject make_recycler_view(JNIEnv* env, jobject ctx) {
    return make_object(env, "androidx/recyclerview/widget/RecyclerView", ctx);
}

jobject make_linear_layout_manager(JNIEnv* env, jobject ctx, jint orient) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/recyclerview/widget/LinearLayoutManager");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;IZ)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject lm = env->NewObject(cls, ctor, ctx, orient, JNI_FALSE);
    env->DeleteLocalRef(cls);
    if (env->ExceptionCheck() || lm == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    jobject g = env->NewGlobalRef(lm);
    env->DeleteLocalRef(lm);
    return g;
}

jobject make_grid_layout_manager(JNIEnv* env, jobject ctx, jint span, jint orient) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("androidx/recyclerview/widget/GridLayoutManager");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(Landroid/content/Context;IIZ)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject lm = env->NewObject(cls, ctor, ctx, span, orient, JNI_FALSE);
    env->DeleteLocalRef(cls);
    if (env->ExceptionCheck() || lm == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    jobject g = env->NewGlobalRef(lm);
    env->DeleteLocalRef(lm);
    return g;
}

void recycler_set_layout_manager(JNIEnv* env, jobject rv, jobject lm) {
    if (rv == nullptr || lm == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass rv_cls = env->FindClass("androidx/recyclerview/widget/RecyclerView");
    if (rv_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(rv_cls, "setLayoutManager",
        "(Landroidx/recyclerview/widget/RecyclerView$LayoutManager;)V");
    if (m != nullptr) {
        env->CallVoidMethod(rv, m, lm);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(rv_cls);
}

void recycler_set_adapter(JNIEnv* env, jobject rv, jobject adapter) {
    if (rv == nullptr || adapter == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass rv_cls = env->FindClass("androidx/recyclerview/widget/RecyclerView");
    if (rv_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(rv_cls, "setAdapter",
        "(Landroidx/recyclerview/widget/RecyclerView$Adapter;)V");
    if (m != nullptr) {
        env->CallVoidMethod(rv, m, adapter);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(rv_cls);
}

jobject make_collection_adapter(JNIEnv* env, jlong owner_ptr) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("io/mpapp/MppCollectionAdapter");
    if (cls == nullptr) { env->ExceptionClear(); return nullptr; }
    jmethodID ctor = env->GetMethodID(cls, "<init>", "(J)V");
    if (ctor == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(cls); return nullptr; }
    jobject local = env->NewObject(cls, ctor, owner_ptr);
    env->DeleteLocalRef(cls);
    if (env->ExceptionCheck() || local == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    jobject g = env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
    return g;
}

void adapter_set_strings(JNIEnv* env, jobject adapter, const std::vector<std::string>& v) {
    if (adapter == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass str_cls = env->FindClass("java/lang/String");
    if (str_cls == nullptr) { env->ExceptionClear(); return; }
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(v.size()), str_cls, nullptr);
    env->DeleteLocalRef(str_cls);
    if (arr == nullptr) { env->ExceptionClear(); return; }
    for (jsize i = 0; i < static_cast<jsize>(v.size()); ++i) {
        jstring s = env->NewStringUTF(v[static_cast<std::size_t>(i)].c_str());
        env->SetObjectArrayElement(arr, i, s);
        env->DeleteLocalRef(s);
    }
    jclass ad_cls = env->FindClass("io/mpapp/MppCollectionAdapter");
    if (ad_cls != nullptr) {
        jmethodID m = env->GetMethodID(ad_cls, "setStrings", "([Ljava/lang/String;)V");
        if (m != nullptr) {
            env->CallVoidMethod(adapter, m, arr);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(ad_cls);
    }
    env->DeleteLocalRef(arr);
}

void adapter_set_native_views(JNIEnv* env, jobject adapter, const std::vector<view*>& items) {
    if (adapter == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass view_cls = env->FindClass("android/view/View");
    if (view_cls == nullptr) { env->ExceptionClear(); return; }
    jobjectArray arr = env->NewObjectArray(static_cast<jsize>(items.size()), view_cls, nullptr);
    env->DeleteLocalRef(view_cls);
    if (arr == nullptr) { env->ExceptionClear(); return; }
    for (jsize i = 0; i < static_cast<jsize>(items.size()); ++i) {
        view* item = items[static_cast<std::size_t>(i)];
        if (item == nullptr) continue;
        jobject native_item = detail::android_dispatch::dispatch(item);
        if (native_item != nullptr) {
            env->SetObjectArrayElement(arr, i, native_item);
        }
    }
    jclass ad_cls = env->FindClass("io/mpapp/MppCollectionAdapter");
    if (ad_cls != nullptr) {
        jmethodID m = env->GetMethodID(ad_cls, "setNativeViews", "([Landroid/view/View;)V");
        if (m != nullptr) {
            env->CallVoidMethod(adapter, m, arr);
            if (env->ExceptionCheck()) env->ExceptionClear();
        }
        env->DeleteLocalRef(ad_cls);
    }
    env->DeleteLocalRef(arr);
}

void adapter_set_selection_mode(JNIEnv* env, jobject adapter, collection_selection_mode m) {
    if (adapter == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass ad_cls = env->FindClass("io/mpapp/MppCollectionAdapter");
    if (ad_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID f = env->GetMethodID(ad_cls, "setSelectionMode", "(I)V");
    if (f != nullptr) {
        jint mode = 1; // SINGLE
        switch (m) {
            case collection_selection_mode::none:     mode = 0; break;
            case collection_selection_mode::multiple: mode = 2; break;
            case collection_selection_mode::single:
            default:                                  mode = 1; break;
        }
        env->CallVoidMethod(adapter, f, mode);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(ad_cls);
}

void adapter_select_index(JNIEnv* env, jobject adapter, jint idx) {
    if (adapter == nullptr) return;
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass ad_cls = env->FindClass("io/mpapp/MppCollectionAdapter");
    if (ad_cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID f = env->GetMethodID(ad_cls, "selectIndex", "(I)V");
    if (f != nullptr) {
        env->CallVoidMethod(adapter, f, idx);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(ad_cls);
}

} // namespace

collection_view_handler<platform::android>::collection_view_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_object(env, "android/widget/FrameLayout", detail::get_activity());
    inner_  = make_recycler_view(env, detail::get_activity());
    if (native_ != nullptr && inner_ != nullptr) {
        framelayout_add_match(env, native_, inner_);
        // Seed with vertical_list LayoutManager; apply_layout will swap
        // it the moment the surface is bound to a different layout.
        jobject lm = make_linear_layout_manager(env, detail::get_activity(), RV_VERTICAL);
        if (lm != nullptr) {
            recycler_set_layout_manager(env, inner_, lm);
            env->DeleteGlobalRef(lm);
        }
    }
    // adapter_ is created lazily once map_items_source / map_typed_items
    // binds a basic_collection_view (we need its address as the ownerPtr).
}

collection_view_handler<platform::android>::~collection_view_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (adapter_ != nullptr) { env->DeleteGlobalRef(adapter_); adapter_ = nullptr; }
        if (inner_   != nullptr) { env->DeleteGlobalRef(inner_);   inner_   = nullptr; }
        if (native_  != nullptr) { env->DeleteGlobalRef(native_);  native_  = nullptr; }
    }
}

namespace {
// One-time: create the adapter (needs bound_->this pointer) and attach
// it to the RecyclerView. Re-basic_entry is a no-op.
void ensure_adapter(JNIEnv* env,
                    jobject inner,
                    jobject& adapter_slot,
                    basic_collection_view* cv) {
    if (env == nullptr || inner == nullptr || cv == nullptr) return;
    if (adapter_slot != nullptr) return;
    adapter_slot = make_collection_adapter(env, reinterpret_cast<jlong>(cv));
    if (adapter_slot != nullptr) {
        recycler_set_adapter(env, inner, adapter_slot);
    }
}
} // namespace

void collection_view_handler<platform::android>::rebuild_items(const std::vector<std::string>& v) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    ensure_adapter(env, inner_, adapter_, bound_);
    adapter_set_strings(env, adapter_, v);
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void collection_view_handler<platform::android>::rebuild_typed(const std::vector<view*>& items) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    ensure_adapter(env, inner_, adapter_, bound_);
    adapter_set_native_views(env, adapter_, items);
    if (bound_ != nullptr) apply_selection(bound_->selected_index.get());
}

void collection_view_handler<platform::android>::rebuild_active() {
    if (bound_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    ensure_adapter(env, inner_, adapter_, bound_);
    // Re-apply selection mode each rebuild so the adapter knows what
    // tap behavior to use (single/multi/none).
    adapter_set_selection_mode(env, adapter_, bound_->selection_mode.get());

    if (!bound_->typed_items.get().empty()) {
        rebuild_typed(bound_->typed_items.get());
    } else if (bound_->materialized_count() > 0) {
        rebuild_typed(bound_->materialized_views());
    } else {
        rebuild_items(bound_->items_source.get());
    }
}

void collection_view_handler<platform::android>::apply_selection(int idx) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    adapter_select_index(env, adapter_, static_cast<jint>(idx));
}

void collection_view_handler<platform::android>::apply_selection_mode(collection_selection_mode m) {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    adapter_set_selection_mode(env, adapter_, m);
}

void collection_view_handler<platform::android>::apply_layout(collection_layout l) {
    if (inner_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;

    jobject ctx = detail::get_activity();
    jobject lm  = nullptr;
    const jint span = (bound_ != nullptr && bound_->span.get() > 1)
                      ? static_cast<jint>(bound_->span.get())
                      : DEFAULT_GRID_SPAN;
    switch (l) {
        case collection_layout::vertical_list:
            lm = make_linear_layout_manager(env, ctx, RV_VERTICAL);
            break;
        case collection_layout::horizontal_list:
            lm = make_linear_layout_manager(env, ctx, RV_HORIZONTAL);
            break;
        case collection_layout::vertical_grid:
            lm = make_grid_layout_manager(env, ctx, span, RV_VERTICAL);
            break;
        case collection_layout::horizontal_grid:
            lm = make_grid_layout_manager(env, ctx, span, RV_HORIZONTAL);
            break;
    }
    if (lm == nullptr) return;
    recycler_set_layout_manager(env, inner_, lm);
    env->DeleteGlobalRef(lm);
}

void collection_view_handler<platform::android>::map_items_source(basic_collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.items_source.changed.subscribe(items_slot_, items_cb_);
}

void collection_view_handler<platform::android>::map_typed_items(basic_collection_view& cv) {
    bound_ = &cv;
    rebuild_active();
    cv.typed_items.changed.subscribe(typed_slot_, typed_cb_);
    cv.materialized_changed.subscribe(materialized_slot_, materialized_cb_);
}

void collection_view_handler<platform::android>::map_selected_index(basic_collection_view& cv) {
    apply_selection(cv.selected_index.get());
    cv.selected_index.changed.subscribe(sel_slot_, sel_cb_);
}

void collection_view_handler<platform::android>::map_selection_mode(basic_collection_view& cv) {
    apply_selection_mode(cv.selection_mode.get());
    cv.selection_mode.changed.subscribe(mode_slot_, mode_cb_);
}

void collection_view_handler<platform::android>::map_layout(basic_collection_view& cv) {
    apply_layout(cv.layout.get());
    cv.layout.changed.subscribe(layout_slot_, layout_cb_);
}

} // namespace mpapp::internal
// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_collection_view(::mpapp::view* v) {
    if (auto* c = dynamic_cast<::mpapp::internal::basic_collection_view*>(v); c && c->has_cv_handler()) {
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
