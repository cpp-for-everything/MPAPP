// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp CLI subcommand entry points + a couple of helpers shared with the
// tests.
//
// Each subcommand exposes a free function with the signature expected by
// `command::run`. `build` and `xaml-compile` invoke real underlying tools
// (cmake, mpapp-xc); the others are still stubs that echo back recognized
// arguments. Set `MPAPP_CLI_DRY_RUN=1` in the environment to make the real
// commands print the would-be invocation and exit 0 without spawning a
// child — used by the integration tests.

#pragma once

#include "mpapp_cli/dispatch.hpp"

#include <filesystem>
#include <iosfwd>
#include <optional>

namespace mpapp::cli::commands {

int run_new(args_span args, std::ostream& out, std::ostream& err);
int run_build(args_span args, std::ostream& out, std::ostream& err);
int run_run(args_span args, std::ostream& out, std::ostream& err);
int run_package(args_span args, std::ostream& out, std::ostream& err);
int run_xaml_compile(args_span args, std::ostream& out, std::ostream& err);
int run_help(args_span args, std::ostream& out, std::ostream& err);

// Walks up from the running executable looking for a `CMakeLists.txt`
// whose first non-blank, non-comment line begins with `project(mpapp`.
// Returns the directory containing that file, or std::nullopt if the
// walk reached the filesystem root without finding it.
std::optional<std::filesystem::path> find_repo_root();

// Locate the directory containing the running mpapp executable.
// Returns an empty path on platforms where we can't determine it.
std::filesystem::path mpapp_exe_dir();

} // namespace mpapp::cli::commands
