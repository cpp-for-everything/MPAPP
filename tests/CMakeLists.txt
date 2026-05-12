include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

add_executable(smoke_test smoke_test.cpp)
target_link_libraries(smoke_test PRIVATE mpapp-core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(smoke_test)
