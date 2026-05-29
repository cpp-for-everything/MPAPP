// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Anchor translation unit for the `mpapp-core` static library.
//
// mpapp-core is deliberately PLATFORM-NEUTRAL (ADR-0024 + T-0032
// Path B). It must NOT include the public umbrella <mpapp/mpapp.hpp>:
// the umbrella pulls in every wrapper (button.hpp, label.hpp, …), and
// after ADR-0024 each wrapper embeds a `<platform::current>` handler by
// value. On Windows that drags in the WinUI 3 / WindowsAppSDK
// projection headers (`winrt/Microsoft.UI.Xaml.Controls.h` …) that
// mpapp-core does not — and by design should not — carry. The real
// per-platform handlers live in the handler libraries that examples and
// apps link explicitly (see CMakeLists.txt: "UI handlers … are NOT
// compiled into mpapp-core because they pull in heavy native SDKs").
//
// Dropping the umbrella include here is what lets `mpapp-core` build on
// a clean Windows runner (and on this machine) without WindowsAppSDK on
// the compiler's include path. Umbrella reachability — i.e. "the full
// public API compiles with the real handler stack" — is validated by
// the example targets, which wire the per-platform SDK include paths.
//
// This TU exists only so the static archive has at least one object
// file; the anchor symbol below guarantees a non-empty member.

namespace mpapp::detail {

// Non-inline definition → emits a real symbol into mpapp-core.lib /
// libmpapp-core.a so the archive is never empty. Never called.
int mpapp_core_anchor() noexcept { return 0; }

} // namespace mpapp::detail
