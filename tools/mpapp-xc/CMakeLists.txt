# `mpapp-xc` — XAML compiler. Builds the parser/emitter library (over pugixml)
# and the CLI front-end.
include(MpappWarnings)

include(FetchContent)
# pugixml — MIT header-only XML parser, vendored so consumers need no system
# install. See vault/70_References/Third-Party Dependencies.md.
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.14
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(pugixml)

add_library(mpapp-xc-lib STATIC
    src/parser.cpp
    src/emitter.cpp
    src/type_registry.cpp
)
target_include_directories(mpapp-xc-lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(mpapp-xc-lib PUBLIC cxx_std_23)
target_link_libraries(mpapp-xc-lib PRIVATE pugixml::pugixml)
mpapp_apply_warnings(mpapp-xc-lib)

add_executable(mpapp-xc main.cpp)
target_link_libraries(mpapp-xc PRIVATE mpapp-xc-lib)
target_compile_features(mpapp-xc PRIVATE cxx_std_23)
mpapp_apply_warnings(mpapp-xc)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
