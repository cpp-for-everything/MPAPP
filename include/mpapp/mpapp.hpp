// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP.
//
// MPAPP umbrella header.
//
// Canonical entry point for the MPAPP public API. Including this header brings
// the entire user-facing surface of the framework into scope via the mpapp
// namespace. The public API is expressed exclusively through C++ types
// (templates, classes, free functions) -- no preprocessor macros are part of
// the public surface (see ADR-0002, ADR-0009).

#pragma once

#include "command.hpp"
#include "computed.hpp"
#include "observable.hpp"
#include "signal.hpp"

namespace mpapp {

// Subsystem headers populate this namespace.

} // namespace mpapp
