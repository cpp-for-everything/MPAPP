// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0011-dependency-injection.md
//
// `mpapp::app_builder` — the host-configuration entry point, MAUI's
// `MauiAppBuilder`. Owns a `service_collection` (exposed via
// `services()`); `build()` produces the resolved `service_provider`.
// Full `mpapp::run<App>` wiring (so the application + handlers resolve
// from the provider) is a follow-up — this keeps the DI surface
// platform-neutral + testable on its own. No macros.

#ifndef MPAPP_DI_APP_BUILDER_HPP
#define MPAPP_DI_APP_BUILDER_HPP

#include <utility>

#include "service_collection.hpp"

namespace mpapp {

class app_builder {
public:
    app_builder() = default;

    // The service registry; chain `.services().add_singleton<…>()` etc.
    [[nodiscard]] service_collection& services() noexcept { return services_; }
    [[nodiscard]] const service_collection& services() const noexcept { return services_; }

    // Finalize: build the resolved provider. Consumes the registrations.
    [[nodiscard]] service_provider build() { return services_.build(); }

private:
    service_collection services_{};
};

} // namespace mpapp

#endif // MPAPP_DI_APP_BUILDER_HPP
