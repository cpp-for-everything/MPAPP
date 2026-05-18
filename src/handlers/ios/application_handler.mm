// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — UIKit application handler implementation.

#include "mpapp/handlers/ios/application_handler.hpp"

#if defined(__APPLE__) && TARGET_OS_IPHONE

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>

namespace mpapp::detail {

// Singleton handoff between `uikit_run_app_impl` and our
// UIApplicationDelegate. UIApplicationMain takes a delegate-class-name
// string, not an instance, so we stash the launcher in a global.
struct uikit_handoff {
    uikit_application_launcher launcher{};
    mpapp::application*        out_app  = nullptr;
};

static uikit_handoff g_uikit_handoff{};

} // namespace mpapp::detail

@interface MppUIKitAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow* window;
@end

@implementation MppUIKitAppDelegate
- (BOOL)application:(UIApplication*)application
        didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
    (void)application;
    (void)launchOptions;
    auto& h = mpapp::detail::g_uikit_handoff;
    if (h.launcher.construct) {
        h.out_app = h.launcher.construct();
        if (h.out_app) {
            h.out_app->on_launch();
        }
    }
    return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application {
    (void)application;
    auto& h = mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_suspend();
}

- (void)applicationDidBecomeActive:(UIApplication*)application {
    (void)application;
    auto& h = mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_resume();
}

- (void)applicationWillTerminate:(UIApplication*)application {
    (void)application;
    auto& h = mpapp::detail::g_uikit_handoff;
    if (h.out_app) h.out_app->on_terminate();
}
@end

namespace mpapp::detail {

int uikit_run_app_impl(const uikit_application_launcher& launcher,
                       int argc, char** argv,
                       mpapp::application*& out_app) {
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
