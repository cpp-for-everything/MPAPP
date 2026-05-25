// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::stream_image_source` — counterpart to MAUI's
// StreamImageSource. The `factory` callable is invoked by the
// per-platform loader when the source is bound to an image control;
// it returns the raw bytes that the loader decodes into a native
// bitmap. Useful for generated content, downloaded payloads kept in
// memory, or proprietary archive formats.

#ifndef MPAPP_IMAGE_SOURCES_STREAM_IMAGE_SOURCE_HPP
#define MPAPP_IMAGE_SOURCES_STREAM_IMAGE_SOURCE_HPP

#include <cstddef>
#include <functional>
#include <vector>

#include "../internal/basic_image_source.hpp"

namespace mpapp {

class stream_image_source : public internal::basic_image_source {
public:
    stream_image_source() = default;
    explicit stream_image_source(std::function<std::vector<std::byte>()> f)
        : factory{std::move(f)} {}

    // ----- Bindable configuration ----------------------------------------
    // Sync byte-producing callable. Real per-platform loaders may
    // wrap this with a thread-pool dispatch + `std::future` so the UI
    // thread doesn't block; the mock loader invokes it inline. An
    // empty factory means "no payload" — the loader reports a no-op.
    std::function<std::vector<std::byte>()> factory{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::stream;
    }
};

} // namespace mpapp

#endif // MPAPP_IMAGE_SOURCES_STREAM_IMAGE_SOURCE_HPP
