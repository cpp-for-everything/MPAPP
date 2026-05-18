// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — WinUI 3 application handler implementation.
//
// This translation unit owns the entire Windows-side bootstrap goo so
// the user-facing surface (mpapp/application.hpp, mpapp/run.hpp) never
// names `MddBootstrap*`, `winrt::init_apartment`, `Application::Start`,
// or `mux::ApplicationT<App>`.

#include "mpapp/handlers/windows/application_handler.hpp"

#if defined(_WIN32)

#include <atomic>
#include <stdexcept>
#include <string>

#include <windows.h>
#include <MddBootstrap.h>

extern "C" HRESULT __stdcall WindowsAppRuntime_EnsureIsLoaded();

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace mpapp::detail {

namespace mux = ::winrt::Microsoft::UI::Xaml;

namespace {

// File-static singleton handoff between `run_app_impl` and the
// `mpapp_winui_app` inner class's `OnLaunched`. Application::Start
// doesn't expose a context pointer, so we stash the launcher + result
// here and the inner App reads them on the UI thread.
struct launcher_handoff {
    application_launcher        launcher{};
    mpapp::application*         out_app    = nullptr;
    std::string                 fatal_msg  = {};
    HRESULT                     fatal_hr   = S_OK;
};

std::atomic<launcher_handoff*> g_handoff{nullptr};

class mpapp_winui_app : public mux::ApplicationT<mpapp_winui_app> {
public:
    void OnLaunched(mux::LaunchActivatedEventArgs const&) {
        auto* h = g_handoff.load();
        if (h == nullptr || h->launcher.construct == nullptr) {
            return;
        }
        try {
            h->out_app = h->launcher.construct();
            if (h->out_app != nullptr) {
                h->out_app->on_launch();
            }
        } catch (winrt::hresult_error const& e) {
            h->fatal_hr  = e.code().value;
            h->fatal_msg = "winrt::hresult_error in on_launch";
        } catch (std::exception const& e) {
            h->fatal_msg = std::string{"std::exception in on_launch: "} + e.what();
            h->fatal_hr  = E_FAIL;
        } catch (...) {
            h->fatal_msg = "unknown exception in on_launch";
            h->fatal_hr  = E_FAIL;
        }
    }
};

} // namespace

int run_app_impl(const application_launcher& launcher,
                 int /*argc*/, char** /*argv*/,
                 mpapp::application*& out_app) {
    // 1. Bootstrap the unpackaged Windows App SDK 1.8 dynamic dependency.
    PACKAGE_VERSION min_version{};
    if (const HRESULT hr = ::MddBootstrapInitialize2(
            0x00010008, nullptr, min_version,
            MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
        FAILED(hr)) {
        return static_cast<int>(hr);
    }

    if (const HRESULT hr = ::WindowsAppRuntime_EnsureIsLoaded(); FAILED(hr)) {
        ::MddBootstrapShutdown();
        return static_cast<int>(hr);
    }

    // 2. Initialize the STA apartment WinUI 3 requires.
    int rc = 0;
    try {
        ::winrt::init_apartment(::winrt::apartment_type::single_threaded);

        // 3. Hand the launcher off to the inner App and start the loop.
        launcher_handoff handoff{};
        handoff.launcher = launcher;
        g_handoff.store(&handoff);

        mux::Application::Start(
            []([[maybe_unused]] mux::ApplicationInitializationCallbackParams const& p) {
                ::winrt::make<mpapp_winui_app>();
            });

        g_handoff.store(nullptr);
        out_app = handoff.out_app;
        rc      = SUCCEEDED(handoff.fatal_hr) ? 0 : static_cast<int>(handoff.fatal_hr);
    } catch (winrt::hresult_error const& e) {
        rc = static_cast<int>(e.code().value);
    } catch (...) {
        rc = E_FAIL;
    }

    ::MddBootstrapShutdown();
    return rc;
}

} // namespace mpapp::detail

#endif // _WIN32
