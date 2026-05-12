// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0003 — WinUI 3 button spike.
//
// Minimal WinUI 3 desktop app that demonstrates the MPAPP handler
// pattern end-to-end:
//
//   1. Initialise the unpackaged Windows App SDK via MddBootstrap.
//   2. Force-load Microsoft.WindowsAppRuntime.dll so the undocked
//      reg-free WinRT manifest is registered before Application::Start.
//   3. Subclass `winrt::Microsoft::UI::Xaml::Application` in-place.
//   4. In the App's OnLaunched (which runs on the UI thread), construct
//      the MPAPP widgets + handlers, set their initial state, and add
//      the native widgets to a Window.
//   5. Click handler increments a `Observable<int> count`; a slot on
//      `count.changed` writes the new value into `label.text` — which
//      propagates to the native TextBlock through the label handler.
//
// No public-API macros are used. Native WinUI types appear only in the
// App scaffolding; the property/event flow goes entirely through MPAPP
// cross-platform surface.

#include <windows.h>

#include <string>

#include <MddBootstrap.h>

// Exported by Microsoft.WindowsAppRuntime.dll. Calling it registers the
// undocked reg-free WinRT manifest with the loader, which is what makes
// `Application::Start` able to activate WinUI 3 types in an unpackaged
// app. Without this, Start throws RPC_E_WRONG_THREAD. See
// `vault/50_Tasks/T-0003-winui3-button-spike/notes/rpc-e-wrong-thread.md`.
extern "C" HRESULT __stdcall WindowsAppRuntime_EnsureIsLoaded();

#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
// Click lives on IButtonBase, defined in the Primitives header.
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>

#include <mpapp/button.hpp>
#include <mpapp/label.hpp>
#include <mpapp/observable.hpp>
#include <mpapp/signal.hpp>
#include <mpapp/handlers/windows/button_handler.hpp>
#include <mpapp/handlers/windows/label_handler.hpp>

namespace mux  = ::winrt::Microsoft::UI::Xaml;
namespace muxc = ::winrt::Microsoft::UI::Xaml::Controls;

namespace {

// View-model for the spike — a single Observable<int> `count`.
struct view_model {
    mpapp::Observable<int> count{0};
};

// State owned by the App. Lives for the lifetime of the WinUI App so
// the MPAPP widgets, handlers, and signal slots are stable references.
struct spike_state {
    view_model                                       vm;
    mpapp::button                                    btn;
    mpapp::label                                     lbl;
    mpapp::button_handler<mpapp::platform::windows>  btn_handler;
    mpapp::label_handler<mpapp::platform::windows>   lbl_handler;
    mpapp::signal_slot<>                             click_slot;
    mpapp::signal_slot<const int&>                   count_slot;

    // signal::subscribe holds the callable by reference, so we keep the
    // trampolines as members rather than passing temporaries.
    struct click_callback {
        spike_state* self;
        void operator()() const {
            self->vm.count.set(self->vm.count.get() + 1);
        }
    } click_cb{this};

    struct count_callback {
        spike_state* self;
        void operator()(int n) const {
            self->lbl.text.set("Count: " + std::to_string(n));
        }
    } count_cb{this};
};

// A "real" WinUI 3 app would implement IXamlMetadataProvider here so the
// runtime can resolve theme/control types at startup. The T-0003 spike
// builds the UI tree fully programmatically without XAML resource
// lookups, so we elide that interface for now.
struct App : mux::ApplicationT<App> {
    // spike_state owns native WinUI widgets. Constructing it here makes
    // those widgets come into being on the UI thread that the App is
    // instantiated on. Creating them earlier (in wWinMain) throws
    // RPC_E_WRONG_THREAD because the WinUI dispatcher isn't set up yet.
    spike_state  state{};
    mux::Window  window{nullptr};

    void OnLaunched(mux::LaunchActivatedEventArgs const&) {
        // Wire MPAPP handlers to their cross-platform widgets.
        state.btn.set_handler(state.btn_handler);
        state.lbl.set_handler(state.lbl_handler);

        // Initial property values via the MPAPP surface.
        state.btn.text = "Click me";
        state.lbl.text = "Count: 0";

        // Push initial values into native widgets and wire change-signals.
        state.btn_handler.map_text(state.btn);
        state.lbl_handler.map_text(state.lbl);
        state.btn_handler.map_clicked(state.btn);

        // Click increments `count` via the cross-platform signal.
        state.btn.clicked.subscribe(state.click_slot, state.click_cb);

        // count changes drive label text changes via MPAPP only — the
        // native TextBlock is updated by the label handler's text mapper.
        state.vm.count.changed.subscribe(state.count_slot, state.count_cb);

        muxc::StackPanel panel{};
        panel.Orientation(muxc::Orientation::Vertical);
        panel.Spacing(12);
        panel.Padding(mux::Thickness{24, 24, 24, 24});
        panel.HorizontalAlignment(mux::HorizontalAlignment::Center);
        panel.VerticalAlignment(mux::VerticalAlignment::Center);
        panel.Children().Append(state.lbl_handler.native());
        panel.Children().Append(state.btn_handler.native());

        window = mux::Window{};
        window.Title(L"MPAPP T-0003 - WinUI 3 button spike");
        window.Content(panel);
        window.Activate();
    }
};

// Show a message-box and return non-zero from main so the failure is
// visible without a debugger. Used only on the fatal-error path; the
// happy path is silent.
int fail(wchar_t const* what, HRESULT hr) {
    wchar_t buf[256];
    ::swprintf_s(buf, L"%s failed: 0x%08X", what, static_cast<unsigned>(hr));
    ::MessageBoxW(nullptr, buf, L"MPAPP spike", MB_OK | MB_ICONERROR);
    return 1;
}

} // namespace

int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    // Bootstrap the unpackaged Windows App SDK 1.8 dynamic dependency.
    // 0x0001'0008 = major.minor 1.8; the matching framework package
    // ships preinstalled on Windows 11 24H2 development images.
    PACKAGE_VERSION min_version{};
    if (const HRESULT hr = ::MddBootstrapInitialize2(
            0x00010008, nullptr, min_version,
            MddBootstrapInitializeOptions_OnNoMatch_ShowUI);
        FAILED(hr)) {
        return fail(L"MddBootstrapInitialize2", hr);
    }

    if (const HRESULT hr = ::WindowsAppRuntime_EnsureIsLoaded(); FAILED(hr)) {
        ::MddBootstrapShutdown();
        return fail(L"WindowsAppRuntime_EnsureIsLoaded", hr);
    }

    try {
        // WinUI 3 requires an STA. Application::Start sets up the UI
        // dispatcher on top of it.
        ::winrt::init_apartment(::winrt::apartment_type::single_threaded);

        mux::Application::Start(
            []([[maybe_unused]] mux::ApplicationInitializationCallbackParams const& p) {
                ::winrt::make<App>();
            });
    } catch (::winrt::hresult_error const& e) {
        ::MddBootstrapShutdown();
        return fail(L"Application::Start", e.code().value);
    }

    ::MddBootstrapShutdown();
    return 0;
}
