// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock loader for the RFC-0004 image-source family.
//
// `mpapp::image_loader<platform::mock>` records every `load(source)`
// invocation into the `mock_handler_base::calls()` log so tests can
// assert which sources would have been loaded + in what order. Does
// NOT actually decode anything — the mock simply captures the request
// shape (`load(<kind>)=<key>`) per source kind.

#ifndef MPAPP_HANDLERS_MOCK_IMAGE_LOADER_HPP
#define MPAPP_HANDLERS_MOCK_IMAGE_LOADER_HPP

#include <chrono>
#include <string>

#include "../../image_sources/file_image_source.hpp"
#include "../../image_sources/font_image_source.hpp"
#include "../../image_sources/resource_image_source.hpp"
#include "../../image_sources/stream_image_source.hpp"
#include "../../image_sources/uri_image_source.hpp"
#include "../../internal/basic_image_source.hpp"
#include "../../platform.hpp"
#include "handler_base.hpp"

namespace mpapp {

template <class Platform>
class image_loader;

template <>
class image_loader<platform::mock> : public mock_handler_base {
public:
    image_loader() = default;

    // Records one entry per call. The entry's tag carries the source
    // kind; the value carries the source-specific key (file path /
    // uri / glyph / resource name / "<bytes>"). Tests assert with
    // `calls_as_strings()`.
    void load(const image_source_ref& src) {
        if (!src) {
            record_event("load(null)");
            return;
        }
        switch (src->kind()) {
            case internal::image_source_kind::file: {
                const auto& s = static_cast<const file_image_source&>(*src);
                record_change("load(file)", s.file.get());
                break;
            }
            case internal::image_source_kind::uri: {
                const auto& s = static_cast<const uri_image_source&>(*src);
                record_change("load(uri)", s.uri.get());
                break;
            }
            case internal::image_source_kind::stream: {
                const auto& s = static_cast<const stream_image_source&>(*src);
                // Invoke the factory so tests that wire a counting
                // factory can observe loader contract.
                if (s.factory) {
                    const auto bytes = s.factory();
                    record_change("load(stream)", std::to_string(bytes.size()));
                } else {
                    record_event("load(stream:no-factory)");
                }
                break;
            }
            case internal::image_source_kind::font: {
                const auto& s = static_cast<const font_image_source&>(*src);
                record_change("load(font)", s.glyph.get());
                break;
            }
            case internal::image_source_kind::resource: {
                const auto& s = static_cast<const resource_image_source&>(*src);
                record_change("load(resource)", s.resource_name.get());
                break;
            }
        }
    }
};

} // namespace mpapp

#endif // MPAPP_HANDLERS_MOCK_IMAGE_LOADER_HPP
