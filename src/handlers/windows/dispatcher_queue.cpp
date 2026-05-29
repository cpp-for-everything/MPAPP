// SPDX-License-Identifier: Apache-2.0
// WinUI 3 real main-thread dispatcher implementation (DispatcherQueue).

#include "mpapp/handlers/windows/dispatcher_queue.hpp"

#if defined(_WIN32)

#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Dispatching.h>

#include "mpapp/executor.hpp"

namespace mpapp::internal {

namespace mud = ::winrt::Microsoft::UI::Dispatching;

class win_dispatcher final : public ::mpapp::dispatcher {
public:
    explicit win_dispatcher(mud::DispatcherQueue queue) noexcept
        : queue_(std::move(queue)) {}

    void post(std::function<void()> work) override {
        if (queue_ == nullptr) return;
        auto fn = std::make_shared<std::function<void()>>(std::move(work));
        queue_.TryEnqueue([fn] { if (*fn) (*fn)(); });
    }

    void post_after(std::chrono::steady_clock::duration delay,
                    std::function<void()> work) override {
        if (queue_ == nullptr) return;
        auto fn = std::make_shared<std::function<void()>>(std::move(work));
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(delay).count();
        if (ms < 0) ms = 0;
        // One-shot DispatcherQueueTimer: a running timer is kept alive by the
        // DispatcherQueue, so the local goes out of scope safely. Stop() on
        // first tick makes the fire exactly-once (mirrors dispatcher::post).
        mud::DispatcherQueueTimer timer = queue_.CreateTimer();
        timer.Interval(std::chrono::milliseconds(ms));
        timer.IsRepeating(false);
        timer.Tick([fn](mud::DispatcherQueueTimer const& t,
                        ::winrt::Windows::Foundation::IInspectable const&) {
            t.Stop();
            if (*fn) (*fn)();
        });
        timer.Start();
    }

private:
    mud::DispatcherQueue queue_{nullptr};
};

} // namespace mpapp::internal

namespace mpapp::detail {

void install_dispatcher_queue_main_dispatcher() {
    auto queue = ::winrt::Microsoft::UI::Dispatching::DispatcherQueue::GetForCurrentThread();
    if (queue == nullptr) return;   // not on a UI thread — keep the default
    static ::mpapp::internal::win_dispatcher inst{queue};
    ::mpapp::install_main_dispatcher(&inst);
}

} // namespace mpapp::detail

#endif // _WIN32
