#pragma once

#include "App.xaml.g.h"

namespace winrt::uiss::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(
            winrt::Microsoft::UI::Xaml::LaunchActivatedEventArgs const& args);
    };
}
