// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — AppKit application handler implementation.
//
// Owns the NSApplication + NSApplicationDelegate dance so user code's
// main() is just `return mpapp::run<MyApp>(argc, argv);`.

#include "mpapp/handlers/macos/application_handler.hpp"

#if defined(__APPLE__) && !TARGET_OS_IPHONE

#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>

@interface MppAppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, assign) mpapp::detail::appkit_application_launcher launcher;
@property (nonatomic, assign) mpapp::application** outAppSlot;
@end

@implementation MppAppDelegate
- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    (void)notification;
    if (_launcher.construct == nullptr) return;
    mpapp::application* app = _launcher.construct();
    if (_outAppSlot) {
        *_outAppSlot = app;
    }
    if (app) {
        app->on_launch();
    }
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
    (void)sender;
    if (_outAppSlot && *_outAppSlot) {
        (*_outAppSlot)->on_terminate();
    }
    return NSTerminateNow;
}
@end

namespace mpapp::detail {

int appkit_run_app_impl(const appkit_application_launcher& launcher,
                        int /*argc*/, char** /*argv*/,
                        mpapp::application*& out_app) {
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];

        MppAppDelegate* delegate = [[MppAppDelegate alloc] init];
        delegate.launcher    = launcher;
        delegate.outAppSlot  = &out_app;
        [app setDelegate:delegate];

        [app activateIgnoringOtherApps:YES];
        [app run];
        return 0;
    }
}

} // namespace mpapp::detail

#endif // __APPLE__ && !TARGET_OS_IPHONE
