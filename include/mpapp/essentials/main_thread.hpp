// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0013-essentials.md
//
// `mpapp::main_thread` — dispatch and query the UI thread. Counterpart
// to MAUI Essentials `MainThread`. Abstract interface + a mock whose
// is_main_thread() result is settable and whose begin_invoke_on_main_thread()
// executes the callback inline (synchronously) and records an invocation
// count. Real per-platform backends (Windows DispatcherQueue, Android
// Looper, GTK main context) implement the same interface and are injected
// via the DI container (RFC-0011). No macros; header-only interface.

#ifndef MPAPP_ESSENTIALS_MAIN_THREAD_HPP
#define MPAPP_ESSENTIALS_MAIN_THREAD_HPP

#include <cstddef>
#include <functional>

namespace mpapp {

class main_thread {
public:
    virtual ~main_thread() = default;

    // Returns true when the calling thread is the application's main (UI) thread.
    [[nodiscard]] virtual bool is_main_thread() const = 0;

    // Enqueues (or directly executes) `action` on the main thread.
    // MAUI's MainThread.BeginInvokeOnMainThread.
    virtual void begin_invoke_on_main_thread(std::function<void()> action) = 0;
};

// Mock implementation: is_main_thread() returns a settable bool (default
// true). begin_invoke_on_main_thread() runs the callback inline and
// increments an invocation counter. Suitable for unit tests and host tools.
class mock_main_thread final : public main_thread {
public:
    mock_main_thread() = default;

    // ---- main_thread interface ----

    [[nodiscard]] bool is_main_thread() const override { return is_main_thread_; }

    void begin_invoke_on_main_thread(std::function<void()> action) override {
        ++invoke_count_;
        if (action) {
            action();
        }
    }

    // ---- Test-drive API ----

    // Set the value returned by is_main_thread().
    void set_is_main_thread(bool value) noexcept { is_main_thread_ = value; }

    // Number of times begin_invoke_on_main_thread() has been called.
    [[nodiscard]] std::size_t invoke_count() const noexcept { return invoke_count_; }

    // Reset the invocation counter to zero.
    void reset_invoke_count() noexcept { invoke_count_ = 0; }

private:
    bool        is_main_thread_{ true };
    std::size_t invoke_count_{ 0 };
};

} // namespace mpapp

#endif // MPAPP_ESSENTIALS_MAIN_THREAD_HPP
