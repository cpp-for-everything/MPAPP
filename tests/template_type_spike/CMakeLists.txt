# Template-wrapper-type spike (T-0002).
#
# This CMakeLists is intended to be consumed as a subdirectory by the root
# project. Catch2 is provided by the parent's tests/CMakeLists.txt; mpapp-core
# (the umbrella include path) is provided by the root CMakeLists.txt.

add_executable(template_type_spike_test test.cpp)

target_link_libraries(template_type_spike_test PRIVATE
    mpapp-core
    Catch2::Catch2WithMain
)

target_compile_options(template_type_spike_test PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX /permissive- /Zc:__cplusplus>
)

catch_discover_tests(template_type_spike_test)
