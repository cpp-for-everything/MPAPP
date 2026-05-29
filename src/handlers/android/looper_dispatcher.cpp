// SPDX-License-Identifier: Apache-2.0
// Android real main-thread dispatcher implementation (Handler on main Looper).

#include "mpapp/handlers/android/looper_dispatcher.hpp"

#if defined(__ANDROID__)

#include <chrono>
#include <functional>
#include <utility>

#include <jni.h>

#include "mpapp/executor.hpp"
#include "mpapp/handlers/android/jni_bridge.hpp"

namespace mpapp::internal {

// Android main-thread dispatcher. post()/post_after() box the work into a
// heap std::function and hand its pointer (as a jlong token) to an
// io.mpapp.MppDispatchRunnable, then Handler.post(/postDelayed) it onto the
// main Looper. MppDispatchRunnable.run() JNI-dispatches nativeRun(token),
// which invokes + frees the closure (see the trampoline below).
class looper_dispatcher final : public ::mpapp::dispatcher {
public:
    // Resolve the main-thread Handler + the Runnable shim class/methods.
    // Returns false if anything is unavailable (keeps the default in place).
    bool init(JNIEnv* env) {
        if (env == nullptr) return false;
        if (env->ExceptionCheck()) env->ExceptionClear();

        jclass looper_cls = env->FindClass("android/os/Looper");
        if (looper_cls == nullptr) { env->ExceptionClear(); return false; }
        jmethodID get_main = env->GetStaticMethodID(
            looper_cls, "getMainLooper", "()Landroid/os/Looper;");
        if (get_main == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(looper_cls); return false; }
        jobject looper = env->CallStaticObjectMethod(looper_cls, get_main);
        env->DeleteLocalRef(looper_cls);
        if (looper == nullptr) { env->ExceptionClear(); return false; }

        jclass handler_cls = env->FindClass("android/os/Handler");
        if (handler_cls == nullptr) { env->ExceptionClear(); env->DeleteLocalRef(looper); return false; }
        jmethodID handler_ctor = env->GetMethodID(
            handler_cls, "<init>", "(Landroid/os/Looper;)V");
        post_id_         = env->GetMethodID(handler_cls, "post", "(Ljava/lang/Runnable;)Z");
        post_delayed_id_ = env->GetMethodID(handler_cls, "postDelayed", "(Ljava/lang/Runnable;J)Z");
        if (handler_ctor == nullptr || post_id_ == nullptr || post_delayed_id_ == nullptr) {
            env->ExceptionClear(); env->DeleteLocalRef(handler_cls); env->DeleteLocalRef(looper);
            return false;
        }
        jobject handler = env->NewObject(handler_cls, handler_ctor, looper);
        env->DeleteLocalRef(handler_cls);
        env->DeleteLocalRef(looper);
        if (handler == nullptr) { env->ExceptionClear(); return false; }
        handler_ = env->NewGlobalRef(handler);
        env->DeleteLocalRef(handler);

        jclass shim = env->FindClass("io/mpapp/MppDispatchRunnable");
        if (shim == nullptr) { env->ExceptionClear(); return false; }
        runnable_ctor_ = env->GetMethodID(shim, "<init>", "(J)V");
        runnable_cls_  = static_cast<jclass>(env->NewGlobalRef(shim));
        env->DeleteLocalRef(shim);
        return handler_ != nullptr && runnable_cls_ != nullptr && runnable_ctor_ != nullptr;
    }

    void post(std::function<void()> work) override {
        enqueue(std::move(work), /*delay_ms=*/0, /*delayed=*/false);
    }

    void post_after(std::chrono::steady_clock::duration delay,
                    std::function<void()> work) override {
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
        if (ms < 0) ms = 0;
        enqueue(std::move(work), static_cast<jlong>(ms), /*delayed=*/true);
    }

private:
    void enqueue(std::function<void()> work, jlong delay_ms, bool delayed) {
        if (handler_ == nullptr) return;
        JNIEnv* env = ::mpapp::detail::attach_current_thread();
        if (env == nullptr) return;
        auto* heap = new std::function<void()>(std::move(work));
        jobject runnable = env->NewObject(runnable_cls_, runnable_ctor_,
                                          reinterpret_cast<jlong>(heap));
        if (runnable == nullptr) {
            if (env->ExceptionCheck()) env->ExceptionClear();
            delete heap;
            return;
        }
        if (delayed) {
            env->CallBooleanMethod(handler_, post_delayed_id_, runnable, delay_ms);
        } else {
            env->CallBooleanMethod(handler_, post_id_, runnable);
        }
        if (env->ExceptionCheck()) env->ExceptionClear();
        env->DeleteLocalRef(runnable);
    }

    jobject   handler_        = nullptr;   // global ref, Handler(mainLooper)
    jclass    runnable_cls_   = nullptr;   // global ref, MppDispatchRunnable
    jmethodID runnable_ctor_  = nullptr;
    jmethodID post_id_        = nullptr;
    jmethodID post_delayed_id_= nullptr;
};

} // namespace mpapp::internal

namespace mpapp::detail {

void install_android_main_dispatcher() {
    static ::mpapp::internal::looper_dispatcher inst;
    static bool ready = false;
    if (!ready) {
        JNIEnv* env = ::mpapp::detail::attach_current_thread();
        ready = inst.init(env);
    }
    if (ready) {
        ::mpapp::install_main_dispatcher(&inst);
    }
}

} // namespace mpapp::detail

// JNI trampoline for MppDispatchRunnable.run(): invoke + free the closure.
extern "C" JNIEXPORT void JNICALL
Java_io_mpapp_MppDispatchRunnable_nativeRun(JNIEnv* /*env*/, jclass /*cls*/,
                                            jlong token) {
    auto* fn = reinterpret_cast<std::function<void()>*>(token);
    if (fn != nullptr) {
        if (*fn) (*fn)();
        delete fn;
    }
}

#endif // __ANDROID__
