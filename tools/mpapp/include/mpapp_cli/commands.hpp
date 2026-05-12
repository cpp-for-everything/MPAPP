// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp CLI subcommand stubs.
//
// Each subcommand exposes a free function with the signature expected by
// `command::run`. They parse their own flags with a hand-rolled parser (no
// third-party CLI dependency for the skeleton) and print a "not yet
// implemented" line that echoes back the recognized arguments.

#pragma once

#include "mpapp_cli/dispatch.hpp"

#include <iosfwd>

namespace mpapp::cli::commands {

int run_new(args_span args, std::ostream& out, std::ostream& err);
int run_build(args_span args, std::ostream& out, std::ostream& err);
int run_run(args_span args, std::ostream& out, std::ostream& err);
int run_package(args_span args, std::ostream& out, std::ostream& err);
int run_xaml_compile(args_span args, std::ostream& out, std::ostream& err);
int run_help(args_span args, std::ostream& out, std::ostream& err);

} // namespace mpapp::cli::commands
