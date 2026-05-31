// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС Windows WinUI 3 App shell.
//
// This is the VS-C++-WinUI-template-shaped App: an App.xaml-backed
// Application (x:Class="uiss.App") whose <XamlControlsResources/> pulls the
// framework theme resources into the app's resources.pri and wires up
// ms-appx:/// resolution — the piece a hand-rolled code-only App lacks, and
// the reason TextBox/the УИСС login wouldn't render (see vault T-0067).
//
// OnLaunched simply hands off to the existing, platform-neutral
// `uiss::uiss_app` (the same mpapp::application driven on Linux + Android);
// every widget handler is reused unchanged.

#include "App.xaml.h"

#include <winrt/Microsoft.UI.Xaml.h>

#include <mpapp/handlers/windows/dispatcher_queue.hpp>

#include "uiss/app.hpp"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::uiss::implementation
{
    App::App()
    {
        InitializeComponent();
    }

    void App::OnLaunched(LaunchActivatedEventArgs const&)
    {
        // Route mpapp::main_dispatcher() onto the real WinUI DispatcherQueue
        // now that we're on the UI thread.
        ::mpapp::detail::install_dispatcher_queue_main_dispatcher();

        // Construct + launch the platform-neutral УИСС application. Kept alive
        // for the process lifetime (the window owns the UI tree).
        static ::uiss::uiss_app s_app;
        s_app.on_launch();
    }
}
