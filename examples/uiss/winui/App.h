// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Shim header for the УИСС WinUI 3 App shell.
//
// cppwinrt's generated module.g.cpp registers the activation factory for the
// `App` runtimeclass and #includes "App.h". For a XAML app the real
// implementation lives in the code-behind App.xaml.h, and the activation
// factory (factory_implementation::App) is what module.g.cpp instantiates.
// cppwinrt also emits a *stub* App.h under "Generated Files\sources\" with a
// deliberate static_assert tripwire + a conflicting stub implementation::App;
// this shim provides the real wiring and is placed earlier on the include path
// so it wins. See vault T-0067.
#pragma once
#include "App.g.h"      // factory_implementation::AppT (cppwinrt projection)
#include "App.xaml.h"   // implementation::App (the real XAML code-behind)

namespace winrt::uiss::factory_implementation
{
    struct App : AppT<App, implementation::App>
    {
    };
}
