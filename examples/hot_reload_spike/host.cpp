// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0010 hot-reload spike — long-running host program.
//
// The host loads user_code.dll via mpapp::hot_reload::runtime, then polls the
// watched source file once per second. When user_code.cpp changes on disk,
// the runtime rebuilds the dll via clang++ and reloads it. The host then
// observes the new behavior without restarting.

#include <chrono>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <thread>

#include <mpapp/hot_reload.hpp>

int main(int argc, char* argv[]) {
    using namespace std::chrono_literals;

    // Allow overriding the watched source path (useful when running the host
    // from a build directory that isn't a sibling of user_code.cpp).
    std::filesystem::path src =
        (argc > 1) ? std::filesystem::path{argv[1]}
                   : std::filesystem::path{"user_code.cpp"};

    if (!std::filesystem::exists(src)) {
        std::cerr << "user_code.cpp not found at: " << src.string() << "\n";
        std::cerr << "Pass the path as argv[1] or run from its directory.\n";
        return 1;
    }

    try {
        mpapp::hot_reload::runtime rt{src};

        if (rt.compute() == nullptr) {
            std::cerr << "initial build/load failed: " << rt.last_error()
                      << "\n";
            return 1;
        }

        std::cout << "[host] watching " << src.string() << "\n";
        std::cout << "[host] edit user_code.cpp and save to trigger reload\n";

        int x = 1;
        for (;;) {
            if (rt.poll()) {
                std::cout << "[reloaded]\n";
            } else if (!rt.last_error().empty()) {
                std::cerr << "[reload error] " << rt.last_error() << "\n";
            }
            auto fn = rt.compute();
            if (fn != nullptr) {
                std::cout << "compute(" << x << ") = " << fn(x) << "\n";
            }
            ++x;
            std::this_thread::sleep_for(1s);
        }
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
}
