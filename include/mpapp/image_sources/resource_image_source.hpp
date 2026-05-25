// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::resource_image_source` — counterpart to MAUI's
// `ImageSource.FromResource(...)`. References an embedded resource
// by name; the per-platform loader looks up the resource in the
// app's resource registry (a per-platform concept: WinUI's resw,
// Android's res/drawable, iOS's asset catalog, Linux's GResource).
//
// V1 keeps the surface minimal — just the resource_name. The
// resource-registry plumbing (asset bundling, name-to-bytes lookup)
// lands with the upcoming Resource Dictionary RFC.

#ifndef MPAPP_IMAGE_SOURCES_RESOURCE_IMAGE_SOURCE_HPP
#define MPAPP_IMAGE_SOURCES_RESOURCE_IMAGE_SOURCE_HPP

#include <string>
#include <utility>

#include "../internal/basic_image_source.hpp"
#include "../observable.hpp"

namespace mpapp {

class resource_image_source : public internal::basic_image_source {
public:
    resource_image_source() = default;
    explicit resource_image_source(std::string name)
        : resource_name{std::move(name)} {}

    // ----- Bindable configuration ----------------------------------------
    // Logical name of the embedded resource. Looked up in the app's
    // resource registry (see the future Resource Dictionary RFC).
    Observable<std::string> resource_name{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::resource;
    }
};

} // namespace mpapp

#endif // MPAPP_IMAGE_SOURCES_RESOURCE_IMAGE_SOURCE_HPP
