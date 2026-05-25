// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::internal::basic_image_source` — abstract polymorphic base
// for the ImageSource family (file / uri / stream / font / resource).
// Mirrors MAUI's `ImageSource` hierarchy but uses shared_ptr-typed
// polymorphism in a `mpapp::image_source_ref` alias so the same
// source can be referenced by multiple consumers without ownership
// ambiguity.
//
// Concrete sources live in `mpapp::` (user-facing configuration
// objects), not in `internal::` — the `internal::` namespace here is
// only for the polymorphic base that consumers store + dispatch on.
//
// ImageSources do NOT use the wrapper-component pattern from ADR-0024:
// they own no native widget and have no embedded handler. They are
// configuration; the per-platform image loader (a separate handler
// concept) is what consumes them.

#ifndef MPAPP_INTERNAL_BASIC_IMAGE_SOURCE_HPP
#define MPAPP_INTERNAL_BASIC_IMAGE_SOURCE_HPP

#include <cstdint>
#include <memory>

namespace mpapp::internal {

// Closed-set discriminator matching MAUI's ImageSource family
// (FileImageSource, UriImageSource, StreamImageSource,
// FontImageSource, ResourceImageSource). The per-platform image
// loader walks the bound `image_source_ref` and dispatches on
// `kind()` to install the matching native bitmap-creation pipeline.
enum class image_source_kind : std::uint8_t {
    file     = 0,
    uri      = 1,
    stream   = 2,
    font     = 3,
    resource = 4,
};

class basic_image_source {
public:
    virtual ~basic_image_source() = default;

    basic_image_source(const basic_image_source&)            = delete;
    basic_image_source& operator=(const basic_image_source&) = delete;
    basic_image_source(basic_image_source&&)                 = delete;
    basic_image_source& operator=(basic_image_source&&)      = delete;

    [[nodiscard]] virtual image_source_kind kind() const noexcept = 0;

protected:
    basic_image_source() = default;
};

} // namespace mpapp::internal

namespace mpapp {

// User-facing type alias. Image-aware components carry an
// `Observable<image_source_ref>` (or `Observable<std::vector<
// image_source_ref>>` for galleries) without naming `internal::`.
using image_source_ref = std::shared_ptr<internal::basic_image_source>;

} // namespace mpapp

#endif // MPAPP_INTERNAL_BASIC_IMAGE_SOURCE_HPP
