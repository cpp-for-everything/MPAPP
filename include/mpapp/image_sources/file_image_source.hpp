// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. See vault/30_RFCs/RFC-0004-image-source-family.md
//
// `mpapp::file_image_source` — counterpart to MAUI's FileImageSource.
// Carries a local file-system path; the per-platform image loader
// reads + decodes the file when the source is bound to an image
// control. MAUI's implicit-string converter (e.g.
// `<Image Source="icon.png"/>`) lowers to a default-constructed
// file_image_source whose `file` Observable is set to the string.

#ifndef MPAPP_IMAGE_SOURCES_FILE_IMAGE_SOURCE_HPP
#define MPAPP_IMAGE_SOURCES_FILE_IMAGE_SOURCE_HPP

#include <string>
#include <utility>

#include "../internal/basic_image_source.hpp"
#include "../observable.hpp"

namespace mpapp {

class file_image_source : public internal::basic_image_source {
public:
    file_image_source() = default;
    explicit file_image_source(std::string file_path)
        : file{std::move(file_path)} {}

    // ----- Bindable configuration ----------------------------------------
    // Absolute or app-relative file path. Empty = no image (loader
    // reports a no-op).
    Observable<std::string> file{};

    // ----- Polymorphic identity ------------------------------------------
    [[nodiscard]] internal::image_source_kind kind() const noexcept override {
        return internal::image_source_kind::file;
    }
};

} // namespace mpapp

#endif // MPAPP_IMAGE_SOURCES_FILE_IMAGE_SOURCE_HPP
