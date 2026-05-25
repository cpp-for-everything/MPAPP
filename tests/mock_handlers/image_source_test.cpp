// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for the RFC-0004 ImageSource family.

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/image_loader.hpp>
#include <mpapp/image_sources/file_image_source.hpp>
#include <mpapp/image_sources/font_image_source.hpp>
#include <mpapp/image_sources/resource_image_source.hpp>
#include <mpapp/image_sources/stream_image_source.hpp>
#include <mpapp/image_sources/uri_image_source.hpp>
#include <mpapp/internal/basic_image_source.hpp>

using namespace mpapp;

TEST_CASE("file_image_source carries a path + reports the right kind",
          "[mock][image_source][file]") {
    file_image_source f{"avatar.png"};
    CHECK(f.kind()      == internal::image_source_kind::file);
    CHECK(f.file.get()  == "avatar.png");

    file_image_source g;
    CHECK(g.kind()      == internal::image_source_kind::file);
    CHECK(g.file.get().empty());
}

TEST_CASE("uri_image_source has MAUI-parity defaults",
          "[mock][image_source][uri]") {
    uri_image_source u{"https://example.com/icon.png"};
    CHECK(u.kind()                == internal::image_source_kind::uri);
    CHECK(u.uri.get()             == "https://example.com/icon.png");
    CHECK(u.caching_enabled.get() == true);
    // MAUI's TimeSpan default is 1 day.
    CHECK(u.cache_validity.get()  == std::chrono::hours{24});
}

TEST_CASE("font_image_source carries glyph + font_family + size + tint",
          "[mock][image_source][font]") {
    font_image_source fi;
    fi.glyph        = std::string{"\xEE\x9E\xA0"};  // U+E7A0
    fi.font_family  = std::string{"MaterialIcons-Regular"};
    fi.size         = 24.0;

    CHECK(fi.kind()            == internal::image_source_kind::font);
    CHECK(fi.glyph.get()       == "\xEE\x9E\xA0");
    CHECK(fi.font_family.get() == "MaterialIcons-Regular");
    CHECK(fi.size.get()        == 24.0);
    // Tint defaults to a default-constructed color (all-zero, alpha 1 per
    // box_view::color's defaults).
}

TEST_CASE("resource_image_source carries a logical name",
          "[mock][image_source][resource]") {
    resource_image_source r{"icons/home"};
    CHECK(r.kind()              == internal::image_source_kind::resource);
    CHECK(r.resource_name.get() == "icons/home");
}

TEST_CASE("stream_image_source factory is invoked at load time",
          "[mock][image_source][stream]") {
    int invocations = 0;
    auto src = std::make_shared<stream_image_source>(
        [&invocations]() -> std::vector<std::byte> {
            ++invocations;
            return {std::byte{0x89}, std::byte{0x50}, std::byte{0x4E},
                    std::byte{0x47}};   // PNG magic (just the first 4 bytes)
        });

    image_loader<platform::mock> loader;
    loader.load(src);

    REQUIRE(invocations == 1);
    REQUIRE(loader.calls_as_strings() == std::vector<std::string>{
        "load(stream)=4",
    });
}

TEST_CASE("mock image_loader records one entry per kind",
          "[mock][image_source][loader]") {
    image_loader<platform::mock> loader;

    loader.load(std::make_shared<file_image_source>("a.png"));
    loader.load(std::make_shared<uri_image_source>("https://e/b.png"));

    auto fi = std::make_shared<font_image_source>();
    fi->glyph = std::string{"\xEE\x9E\xA0"};
    loader.load(fi);

    loader.load(std::make_shared<resource_image_source>("icons/home"));
    loader.load(nullptr);

    REQUIRE(loader.calls_as_strings() == std::vector<std::string>{
        "load(file)=a.png",
        "load(uri)=https://e/b.png",
        "load(font)=\xEE\x9E\xA0",
        "load(resource)=icons/home",
        "load(null)",
    });
}

TEST_CASE("stream_image_source with empty factory records no-factory event",
          "[mock][image_source][stream]") {
    image_loader<platform::mock> loader;
    loader.load(std::make_shared<stream_image_source>());

    REQUIRE(loader.calls_as_strings() == std::vector<std::string>{
        "load(stream:no-factory)",
    });
}

TEST_CASE("image_source_ref allows shared ownership across consumers",
          "[mock][image_source]") {
    image_source_ref a = std::make_shared<file_image_source>("x.png");
    image_source_ref b = a;                  // same source, two refs
    CHECK(a.get() == b.get());
    CHECK(a.use_count() == 2);

    image_loader<platform::mock> loader;
    loader.load(a);
    loader.load(b);
    REQUIRE(loader.calls_as_strings() == std::vector<std::string>{
        "load(file)=x.png",
        "load(file)=x.png",
    });
}
