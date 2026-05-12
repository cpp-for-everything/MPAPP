// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// Small cross-platform process-spawning helper for the mpapp CLI.
//
// `spawn` synchronously runs a child process and returns its exit code.
// If `output` is non-null, stdout+stderr of the child are captured into it
// instead of being inherited by the parent process. Argument vectors are
// passed verbatim — quoting is the caller's responsibility, but on Windows
// the underlying _spawnvp handles the typical case of plain whitespace-free
// arguments correctly.
//
// On Windows we use _spawnvp (or _popen for capture); elsewhere we use
// fork/execvp with a pipe. This intentionally stays small — the CLI needs
// "run cmake, return its exit code" and not much else.

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace mpapp::cli::process {

// Run `args[0]` with the remaining elements as argv. Returns the child's
// exit code (negative on spawn failure). If `output` is non-null, the
// child's stdout and stderr are captured into it; otherwise the child
// inherits the parent's stdio.
int spawn(const std::vector<std::string>& args, std::string* output);

// Locate an executable on PATH. Returns the absolute path on success or
// an empty string if not found. On Windows, `.exe` is appended if missing.
std::string which(std::string_view name);

} // namespace mpapp::cli::process
