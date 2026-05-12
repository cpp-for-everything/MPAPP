// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// mpapp CLI dispatcher.
//
// The dispatcher resolves argv[1] against a small in-process registry of
// subcommand descriptors. Each descriptor owns its own argument parser; the
// dispatcher only handles the top-level routing (--version, --help, unknown
// command, no args). Output is written to a caller-provided std::ostream so
// tests can capture it deterministically without touching real stdout.

#pragma once

#include <array>
#include <iosfwd>
#include <string_view>

namespace mpapp::cli {

// Thin wrapper around the argv tail handed to a subcommand. The dispatcher
// strips argv[0] and argv[1] (the program name and the subcommand name) and
// passes the remainder to the subcommand's run() function.
struct args_span {
    int argc = 0;
    char** argv = nullptr;
};

// Per-subcommand descriptor. `run` returns a process exit code.
struct command {
    std::string_view name;
    std::string_view summary;
    std::string_view usage;
    int (*run)(args_span args, std::ostream& out, std::ostream& err);
};

// The static registry of supported subcommands. Populated in commands.cpp.
// Sized to exactly match the implementation; if you add a subcommand, bump
// the array extent both here and in commands.cpp.
extern const std::array<command, 6> registry;

// Top-level dispatcher. Returns a process exit code:
//   0 — success (including help/version).
//   2 — usage error (unknown subcommand, missing required arg, etc.).
//   non-zero — propagated from a subcommand's run().
int dispatch(int argc, char** argv, std::ostream& out, std::ostream& err);

// Top-level help. If `sub` names a known subcommand, prints its usage;
// otherwise prints the global help listing every registered command.
int print_help(std::string_view sub, std::ostream& out);

// Prints the CLI version string.
int print_version(std::ostream& out);

} // namespace mpapp::cli
