// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// `mpapp` CLI entry point. Hands argv straight to the dispatcher, which
// writes user-visible output to std::cout and error output to std::cerr.

#include "mpapp_cli/dispatch.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    return mpapp::cli::dispatch(argc, argv, std::cout, std::cerr);
}
