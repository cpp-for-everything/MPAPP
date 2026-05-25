// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::uri_image_source` — counterpart to MAUI's UriImageSource.
// Remote URL with cache-validity + caching-enabled flags. The
// per-platform image loader fetches the URL, decodes, and (if
// caching is enabled) writes the decoded bitmap to an LRU disk
// cache keyed on the URI + the cache-validity timestamp.

#ifndef MPAPP_IMAGE_SOURCES_URI_IMAGE_SOURCE_HPP
#define MPAPP_IMAGE_SOURCES_URI_IMAGE_SOURCE_HPP

#include <chrono>
#include <string>
#include <utility>

#include "../internal/basic_image_source.hpp"
#include "../observable.hpp"

namespace mpapp {

class uri_image_source : public internal::basic_image_source {
public:
    uri_image_source() = default;
    explicit uri_image_source(std::string uri_string)
        : uri{std::move(uri_string)} {}

    // ----- Bindable configuration ----------------------------------------
    Observable<std::string> uri{};

    // Time-to-live for cached entries. MAUI's default is 1 day; mirror
    // it. `std::chrono::milliseconds` for type safety — the XAML
    // lowering converts MAUI's "01:00:00" TimeSpan literal.
    Observable<std::chrono::milliseconds> cache_validity{
        std::chrono::hours{24}};

    // When false, the loader skips both the disk-cache write AND the
    // disk-cache lookup (always fetches fresh). Useful for one-shot
    // images that should never bloat the cache.
    Observable<bool> caching_enabled{true};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::uri;
    }
};

} // namespace mpapp

#endif // MPAPP_IMAGE_SOURCES_URI_IMAGE_SOURCE_HPP
