include(FetchContent)

# pugixml -- MIT-licensed header-only XML parser. Vendored via FetchContent so
# downstream consumers don't need a system install. See
# vault/70_References/Third-Party Dependencies.md for the license row.
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
)

target_include_directories(mpapp-xc-lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(mpapp-xc-lib PUBLIC cxx_std_23)

target_link_libraries(mpapp-xc-lib PRIVATE pugixml::pugixml)

target_compile_options(mpapp-xc-lib PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

add_executable(mpapp-xc main.cpp)
target_link_libraries(mpapp-xc PRIVATE mpapp-xc-lib)
target_compile_features(mpapp-xc PRIVATE cxx_std_23)

target_compile_options(mpapp-xc PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
