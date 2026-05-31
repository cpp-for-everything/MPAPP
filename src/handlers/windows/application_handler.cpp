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
#include <cstdio>
#include <stdexcept>
#include <string>

#include <windows.h>
#include <appmodel.h>   // GetCurrentPackageFullName / APPMODEL_ERROR_NO_PACKAGE
#include <MddBootstrap.h>

extern "C" HRESULT __stdcall WindowsAppRuntime_EnsureIsLoaded();

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>  // IVector<>::Append (MergedDictionaries)
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>   // XamlControlsResources

#include "mpapp/handlers/windows/dispatcher_queue.hpp"

// Diagnostic log used when MPAPP_LOG_LAUNCH=path is set in the env;
// captures fatal exceptions from `OnLaunched` + `UnhandledException`
// to a file so a headless caller can read them. Off by default.
static void mpapp_diag_log(const char* msg) {
    const char* path = std::getenv("MPAPP_LOG_LAUNCH");
    if (path == nullptr || *path == 0) return;
    FILE* f = nullptr;
    if (fopen_s(&f, path, "ab") == 0 && f != nullptr) {
        std::fprintf(f, "%s\n", msg);
        std::fclose(f);
    }
}

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
    mpapp_winui_app() {
        mpapp_diag_log("mpapp_winui_app: ctor enter");

        // Merge the WinUI control theme resources (themeresources.xaml) into the
        // application resources. In a code-only host (no App.xaml) this MUST be
        // done explicitly — otherwise the first templated control (TextBox, etc.)
        // can't resolve its default style at the deferred layout pass and the app
        // dies with 0x80070002 (FILE_NOT_FOUND) a few hundred ms after launch.
        // This is the C++ equivalent of <XamlControlsResources/> in App.xaml.
        try {
            auto xcr = ::winrt::Microsoft::UI::Xaml::Controls::XamlControlsResources{};
            Resources().MergedDictionaries().Append(xcr);
            mpapp_diag_log("mpapp_winui_app: XamlControlsResources merged");
        } catch (winrt::hresult_error const& e) {
            char buf[256]; std::snprintf(buf, sizeof(buf),
                                          "XamlControlsResources merge FAILED hr=0x%08X",
                                          static_cast<unsigned>(e.code().value));
            mpapp_diag_log(buf);
        } catch (...) {
            mpapp_diag_log("XamlControlsResources merge FAILED (unknown)");
        }

        try {
            // Capture exceptions raised on deferred layout passes / event
            // handlers — without this, an exception on the message loop
            // terminates the process silently a few hundred ms after
            // Window.Activate(). The diagnostic logger only writes when
            // MPAPP_LOG_LAUNCH is set in the env.
            UnhandledException(
                [](winrt::Windows::Foundation::IInspectable const&,
                   mux::UnhandledExceptionEventArgs const& args) {
                    char buf[512];
                    const auto msg = winrt::to_string(args.Message());
                    std::snprintf(buf, sizeof(buf),
                                  "UnhandledException: hr=0x%08X msg=%s",
                                  static_cast<unsigned>(args.Exception().value),
                                  msg.c_str());
                    mpapp_diag_log(buf);
                    args.Handled(true);
                });
        } catch (winrt::hresult_error const& e) {
            char buf[256]; std::snprintf(buf, sizeof(buf),
                                          "ctor UnhandledException subscribe FAILED hr=0x%08X",
                                          static_cast<unsigned>(e.code().value));
            mpapp_diag_log(buf);
        } catch (...) {
            mpapp_diag_log("ctor UnhandledException subscribe FAILED (unknown)");
        }
        mpapp_diag_log("mpapp_winui_app: ctor exit");
    }

    void OnLaunched(mux::LaunchActivatedEventArgs const&) {
        mpapp_diag_log("OnLaunched: enter");
        auto* h = g_handoff.load();
        if (h == nullptr || h->launcher.construct == nullptr) {
            mpapp_diag_log("OnLaunched: handoff missing");
            return;
        }
        try {
            // Route mpapp::main_dispatcher() onto the real WinUI
            // DispatcherQueue now that we're on the UI thread — so
            // async_sleep / ui_task continuations / animation ticks run on
            // the real message loop.
            ::mpapp::detail::install_dispatcher_queue_main_dispatcher();
            mpapp_diag_log("OnLaunched: about to construct app");
            h->out_app = h->launcher.construct();
            mpapp_diag_log("OnLaunched: app constructed");
            if (h->out_app != nullptr) {
                mpapp_diag_log("OnLaunched: about to call on_launch");
                h->out_app->on_launch();
                mpapp_diag_log("OnLaunched: on_launch returned");
            } else {
                mpapp_diag_log("OnLaunched: construct returned null");
            }
        } catch (winrt::hresult_error const& e) {
            h->fatal_hr  = e.code().value;
            h->fatal_msg = "winrt::hresult_error in on_launch";
            char buf[512];
            const auto msg = winrt::to_string(e.message());
            std::snprintf(buf, sizeof(buf),
                          "OnLaunched winrt::hresult_error: hr=0x%08X msg=%s",
                          static_cast<unsigned>(e.code().value),
                          msg.c_str());
            mpapp_diag_log(buf);
        } catch (std::exception const& e) {
            h->fatal_msg = std::string{"std::exception in on_launch: "} + e.what();
            h->fatal_hr  = E_FAIL;
            mpapp_diag_log(("OnLaunched std::exception: " + std::string{e.what()}).c_str());
        } catch (...) {
            h->fatal_msg = "unknown exception in on_launch";
            h->fatal_hr  = E_FAIL;
            mpapp_diag_log("OnLaunched unknown exception");
        }
    }
};

} // namespace

int run_app_impl(const application_launcher& launcher,
                 int /*argc*/, char** /*argv*/,
                 mpapp::application*& out_app) {
    mpapp_diag_log("run_app_impl: enter");

    // 1. Bootstrap the Windows App SDK 1.8 dynamic dependency — but ONLY when
    //    running unpackaged. A packaged app (one with package identity) gets
    //    the WindowsAppRuntime framework + its resources via its manifest
    //    <PackageDependency>; calling the unpackaged bootstrap there is wrong
    //    and makes activation bail. Detect identity via GetCurrentPackageFullName
    //    (APPMODEL_ERROR_NO_PACKAGE ⇒ unpackaged). This is also what makes
    //    resource-heavy controls (TextBox/themeresources) resolve when the app
    //    is packaged — the package graph supplies ms-appx:///Microsoft.UI.Xaml/.
    UINT32 pkg_name_len = 0;
    const bool has_identity =
        ::GetCurrentPackageFullName(&pkg_name_len, nullptr) != APPMODEL_ERROR_NO_PACKAGE;
    if (has_identity) {
        mpapp_diag_log("run_app_impl: packaged (identity present) — skipping bootstrap");
    } else {
        PACKAGE_VERSION min_version{};
        if (const HRESULT hr = ::MddBootstrapInitialize2(
                0x00010008, nullptr, min_version,
                MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
            FAILED(hr)) {
            char buf[128]; std::snprintf(buf, sizeof(buf), "MddBootstrapInitialize2 FAILED 0x%08X", static_cast<unsigned>(hr));
            mpapp_diag_log(buf);
            return static_cast<int>(hr);
        }

        if (const HRESULT hr = ::WindowsAppRuntime_EnsureIsLoaded(); FAILED(hr)) {
            ::MddBootstrapShutdown();
            char buf[128]; std::snprintf(buf, sizeof(buf), "WindowsAppRuntime_EnsureIsLoaded FAILED 0x%08X", static_cast<unsigned>(hr));
            mpapp_diag_log(buf);
            return static_cast<int>(hr);
        }
    }

    mpapp_diag_log("run_app_impl: bootstrap done");

    // 2. Initialize the STA apartment WinUI 3 requires.
    int rc = 0;
    try {
        ::winrt::init_apartment(::winrt::apartment_type::single_threaded);
        mpapp_diag_log("run_app_impl: init_apartment done");

        // 3. Hand the launcher off to the inner App and start the loop.
        launcher_handoff handoff{};
        handoff.launcher = launcher;
        g_handoff.store(&handoff);

        mpapp_diag_log("run_app_impl: calling Application::Start");
        mux::Application::Start(
            []([[maybe_unused]] mux::ApplicationInitializationCallbackParams const& p) {
                mpapp_diag_log("Application::Start callback");
                ::winrt::make<mpapp_winui_app>();
            });
        mpapp_diag_log("run_app_impl: Application::Start returned");

        g_handoff.store(nullptr);
        out_app = handoff.out_app;
        rc      = SUCCEEDED(handoff.fatal_hr) ? 0 : static_cast<int>(handoff.fatal_hr);
    } catch (winrt::hresult_error const& e) {
        char buf[256]; std::snprintf(buf, sizeof(buf),
                                     "run_app_impl winrt::hresult_error: hr=0x%08X",
                                     static_cast<unsigned>(e.code().value));
        mpapp_diag_log(buf);
        rc = static_cast<int>(e.code().value);
    } catch (...) {
        mpapp_diag_log("run_app_impl unknown exception");
        rc = E_FAIL;
    }

    if (!has_identity) {
        ::MddBootstrapShutdown();
    }
    mpapp_diag_log("run_app_impl: exit");
    return rc;
}

} // namespace mpapp::detail

#endif // _WIN32
