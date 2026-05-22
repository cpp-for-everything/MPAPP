// SPDX-License-Identifier: Apache-2.0
// Android grid_layout handler implementation.

#include "mpapp/handlers/android/grid_layout_handler.hpp"

#if defined(__ANDROID__)

#include "mpapp/handlers/android/jni_bridge.hpp"
#include "mpapp/handlers/android/widget_dispatch.hpp"

namespace mpapp {

namespace {

jobject make_grid_layout(JNIEnv* env, jobject context) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/GridLayout");
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

void grid_layout_set_int(JNIEnv* env, jobject grid, const char* method_name, jint v) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/widget/GridLayout");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, method_name, "(I)V");
    if (m != nullptr) {
        env->CallVoidMethod(grid, m, v);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

// Build a GridLayout.LayoutParams{row, column, rowSpan, columnSpan}
// and pass it to grid.addView(child, lp).
//
// The simplest construction path: invoke the static
// GridLayout.spec(int start, int size) -> GridLayout.Spec twice (for
// row + column), then construct GridLayout$LayoutParams(rowSpec, colSpec).
jobject make_layout_params(JNIEnv* env, int row, int col, int row_span, int col_span) {
    if (env->ExceptionCheck()) env->ExceptionClear();

    jclass grid_cls = env->FindClass("android/widget/GridLayout");
    if (grid_cls == nullptr) { env->ExceptionClear(); return nullptr; }

    // GridLayout.spec(int start, int size) -> GridLayout.Spec
    jmethodID spec_static = env->GetStaticMethodID(grid_cls, "spec",
        "(II)Landroid/widget/GridLayout$Spec;");
    if (spec_static == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(grid_cls); return nullptr; }

    jobject row_spec = env->CallStaticObjectMethod(grid_cls, spec_static, row, row_span);
    if (env->ExceptionCheck() || row_spec == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(grid_cls);
        return nullptr;
    }
    jobject col_spec = env->CallStaticObjectMethod(grid_cls, spec_static, col, col_span);
    if (env->ExceptionCheck() || col_spec == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(row_spec);
        env->DeleteLocalRef(grid_cls);
        return nullptr;
    }
    env->DeleteLocalRef(grid_cls);

    // GridLayout$LayoutParams(Spec rowSpec, Spec columnSpec)
    jclass lp_cls = env->FindClass("android/widget/GridLayout$LayoutParams");
    if (lp_cls == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(row_spec);
        env->DeleteLocalRef(col_spec);
        return nullptr;
    }
    jmethodID lp_ctor = env->GetMethodID(lp_cls, "<init>",
        "(Landroid/widget/GridLayout$Spec;Landroid/widget/GridLayout$Spec;)V");
    if (lp_ctor == nullptr) {
        env->ExceptionClear();
        env->DeleteLocalRef(row_spec);
        env->DeleteLocalRef(col_spec);
        env->DeleteLocalRef(lp_cls);
        return nullptr;
    }
    jobject lp = env->NewObject(lp_cls, lp_ctor, row_spec, col_spec);
    env->DeleteLocalRef(row_spec);
    env->DeleteLocalRef(col_spec);
    env->DeleteLocalRef(lp_cls);
    if (env->ExceptionCheck() || lp == nullptr) {
        env->ExceptionClear();
        return nullptr;
    }
    return lp;
}

void grid_add_view(JNIEnv* env, jobject grid, jobject child, jobject lp) {
    if (env->ExceptionCheck()) env->ExceptionClear();
    jclass cls = env->FindClass("android/view/ViewGroup");
    if (cls == nullptr) { env->ExceptionClear(); return; }
    jmethodID m = env->GetMethodID(cls, "addView",
        "(Landroid/view/View;Landroid/view/ViewGroup$LayoutParams;)V");
    if (m != nullptr) {
        env->CallVoidMethod(grid, m, child, lp);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    env->DeleteLocalRef(cls);
}

} // namespace

grid_layout_handler<platform::android>::grid_layout_handler() {
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    native_ = make_grid_layout(env, detail::get_activity());
}

grid_layout_handler<platform::android>::~grid_layout_handler() {
    if (JNIEnv* env = detail::attach_current_thread(); env != nullptr) {
        if (native_ != nullptr) { env->DeleteGlobalRef(native_); native_ = nullptr; }
    }
}

void grid_layout_handler<platform::android>::apply_row_count(int n) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    grid_layout_set_int(env, native_, "setRowCount", n > 0 ? n : 1);
}

void grid_layout_handler<platform::android>::apply_column_count(int n) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    grid_layout_set_int(env, native_, "setColumnCount", n > 0 ? n : 1);
}

void grid_layout_handler<platform::android>::apply_row_spacing(double /*s*/) {
    // GridLayout doesn't ship a setRowSpacing-equivalent; spacing
    // typically lives on per-child margins. v1 leaves it untouched.
}

void grid_layout_handler<platform::android>::apply_column_spacing(double /*s*/) {
    // Same as apply_row_spacing.
}

void grid_layout_handler<platform::android>::add_child(grid_layout& g, view& child) {
    if (native_ == nullptr) return;
    JNIEnv* env = detail::attach_current_thread();
    if (env == nullptr) return;
    jobject n = detail::android_dispatch::dispatch(&child);
    if (n == nullptr) return;

    const auto p = g.get_placement(child);
    jobject lp = make_layout_params(env, p.row, p.column, p.row_span, p.column_span);
    if (lp == nullptr) return;
    grid_add_view(env, native_, n, lp);
    env->DeleteLocalRef(lp);
}

void grid_layout_handler<platform::android>::map_row_definitions(grid_layout& g) {
    apply_row_count(static_cast<int>(g.row_definitions.get().size()));
    g.row_definitions.changed.subscribe(rows_slot_, rows_cb_);
}

void grid_layout_handler<platform::android>::map_column_definitions(grid_layout& g) {
    apply_column_count(static_cast<int>(g.column_definitions.get().size()));
    g.column_definitions.changed.subscribe(cols_slot_, cols_cb_);
}

void grid_layout_handler<platform::android>::map_row_spacing(grid_layout& g) {
    apply_row_spacing(g.row_spacing.get());
    g.row_spacing.changed.subscribe(rsp_slot_, rsp_cb_);
}

void grid_layout_handler<platform::android>::map_column_spacing(grid_layout& g) {
    apply_column_spacing(g.column_spacing.get());
    g.column_spacing.changed.subscribe(csp_slot_, csp_cb_);
}

} // namespace mpapp

// ---------- Self-registration --------------------------------------------
namespace {

jobject dispatch_grid_layout(::mpapp::view* v) {
    if (auto* g = dynamic_cast<::mpapp::grid_layout*>(v); g && g->has_handler()) {
        return g->handler().native();
    }
    return nullptr;
}

struct registrar {
    registrar() { ::mpapp::detail::android_dispatch::register_dispatcher(dispatch_grid_layout); }
};

[[maybe_unused]] registrar _reg;

} // namespace

#endif // __ANDROID__
