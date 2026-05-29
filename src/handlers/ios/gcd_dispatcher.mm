// SPDX-License-Identifier: Apache-2.0
// UIKit real main-thread dispatcher implementation (GCD main queue).
//
// BLIND WRITE — compiled + run on a Mac: PENDING (no Apple host/SDK on the
// dev machine). Uses libdispatch, which is identical on macOS + iOS; the two
// .mm files are kept separate to match the per-platform handler layout.

#include "mpapp/handlers/ios/gcd_dispatcher.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#include <chrono>
#include <functional>
#include <memory>
#include <utility>

#import <dispatch/dispatch.h>

#include "mpapp/executor.hpp"

namespace mpapp::internal {

class ios_gcd_dispatcher final : public ::mpapp::dispatcher {
public:
    void post(std::function<void()> work) override {
        auto fn = std::make_shared<std::function<void()>>(std::move(work));
        dispatch_async(dispatch_get_main_queue(), ^{ if (*fn) (*fn)(); });
    }

    void post_after(std::chrono::steady_clock::duration delay,
                    std::function<void()> work) override {
        auto fn = std::make_shared<std::function<void()>>(std::move(work));
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(delay).count();
        if (ns < 0) ns = 0;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, static_cast<int64_t>(ns)),
                       dispatch_get_main_queue(), ^{ if (*fn) (*fn)(); });
    }
};

} // namespace mpapp::internal

namespace mpapp::detail {

void install_ios_main_dispatcher() {
    static ::mpapp::internal::ios_gcd_dispatcher inst;
    ::mpapp::install_main_dispatcher(&inst);
}

} // namespace mpapp::detail

#endif // __APPLE__ && TARGET_OS_IPHONE
