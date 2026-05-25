// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. Mock tests for `mpapp::image::source_object` —
// RFC-0004's rich-source path on the existing `image` control.

#include <memory>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <mpapp/handlers/mock/image_handler.hpp>
#include <mpapp/image_sources/file_image_source.hpp>
#include <mpapp/image_sources/uri_image_source.hpp>
#include <mpapp/internal/basic_image.hpp>
#include <mpapp/internal/basic_image_source.hpp>

using namespace mpapp;

TEST_CASE("basic_image has both source (string) and source_object (ref)",
          "[mock][image][source_object]") {
    internal::basic_image img;
    // Backward-compat string path defaults to empty.
    CHECK(img.source.get().empty());
    // RFC-0004 rich-source path defaults to a null ref.
    CHECK(img.source_object.get() == nullptr);
}

TEST_CASE("map_source_object records the initial value on bind",
          "[mock][image][source_object]") {
    internal::basic_image img;
    img.source_object = std::make_shared<file_image_source>("home.png");

    internal::image_handler<platform::mock> h;
    h.map_source_object(img);

    REQUIRE(h.loader().calls_as_strings() == std::vector<std::string>{
        "load(file)=home.png",
    });
}

TEST_CASE("map_source_object records null source at bind time",
          "[mock][image][source_object]") {
    internal::basic_image img;   // source_object is nullptr by default

    internal::image_handler<platform::mock> h;
    h.map_source_object(img);

    REQUIRE(h.loader().calls_as_strings() == std::vector<std::string>{
        "load(null)",
    });
}

TEST_CASE("map_source_object fires once per real source_object change",
          "[mock][image][source_object]") {
    internal::basic_image img;

    internal::image_handler<platform::mock> h;
    h.map_source_object(img);                                       // load(null)

    img.source_object = std::make_shared<file_image_source>("a.png"); // load(file)=a.png
    img.source_object = std::make_shared<uri_image_source>("https://e/b.png");
                                                                    // load(uri)=https://e/b.png

    REQUIRE(h.loader().calls_as_strings() == std::vector<std::string>{
        "load(null)",
        "load(file)=a.png",
        "load(uri)=https://e/b.png",
    });
}

TEST_CASE("source (string) + source_object coexist on the surface",
          "[mock][image][source_object]") {
    internal::basic_image img;
    img.source        = "fallback.png";                              // legacy path
    img.source_object = std::make_shared<file_image_source>("rich.png");

    // Both observables hold the values they were assigned; the
    // platform handler's policy (which to prefer when both are set)
    // lives in the real handlers per RFC-0004. The mock just reports
    // both surfaces independently when bound.
    internal::image_handler<platform::mock> h;
    h.map_source(img);
    h.map_source_object(img);

    REQUIRE(h.calls_as_strings() == std::vector<std::string>{
        "source=fallback.png",
    });
    REQUIRE(h.loader().calls_as_strings() == std::vector<std::string>{
        "load(file)=rich.png",
    });
}
