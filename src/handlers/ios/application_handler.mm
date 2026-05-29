// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit basic_application handler implementation.

#include "mpapp/handlers/ios/application_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

#include "mpapp/handlers/ios/gcd_dispatcher.hpp"

namespace mpapp::detail {

// Singleton handoff between `uikit_run_app_impl` and our
// UIApplicationDelegate. UIApplicationMain takes a delegate-class-name
// string, not an instance, so we stash the launcher in a global.
struct uikit_handoff {
    uikit_application_launcher launcher{};
    mpapp::internal::basic_application*        out_app  = nullptr;
};

static uikit_handoff g_uikit_handoff{};

} // namespace mpapp::detail

@interface MppUIKitAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* basic_window;
@end

@implementation MppUIKitAppDelegate
- (BOOL)basic_application:(UIApplication*)basic_application
        didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    (void)basic_application;
    (void)launchOptions;
    auto& h = ::mpapp::detail::g_uikit_handoff;
    if (h.launcher.construct) {
        // Route mpapp::main_dispatcher() onto the real GCD main queue now
        // that we're on the main thread — so async_sleep / ui_task
        // continuations / animation ticks run on real frames.
        ::mpapp::detail::install_ios_main_dispatcher();
        h.out_app = h.launcher.construct();
        if (h.out_app) {
            h.out_app->on_launch();
        }
    }
    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)basic_application {
    (void)basic_application;
    auto& h = ::mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_suspend();
}

- (void)applicationDidBecomeActive:(UIApplication*)basic_application {
    (void)basic_application;
    auto& h = ::mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_resume();
}

- (void)applicationWillTerminate:(UIApplication*)basic_application {
    (void)basic_application;
    auto& h = ::mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_terminate();
}
@end

namespace mpapp::detail {

int uikit_run_app_impl(const uikit_application_launcher& launcher,
                       int argc, char** argv,
                       mpapp::internal::basic_application*& out_app) {
    @autoreleasepool {
        g_uikit_handoff.launcher = launcher;
        const int rc = UIApplicationMain(
            argc, argv, nil,
            NSStringFromClass([MppUIKitAppDelegate class]));
        out_app = g_uikit_handoff.out_app;
        return rc;
    }
}

} // namespace mpapp::detail

#endif // __APPLE__ && TARGET_OS_IPHONE
