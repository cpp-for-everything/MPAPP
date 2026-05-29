// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. УИСС — cross-platform "Е-Студент" reference app.
//
// A single, ifdef-free MPAPP application replicating the TU-Sofia
// E-Student student portal (examples/УИСС/*.mhtml). `mpapp::run<App>`
// binds the platform handler set (WinUI 3 / GTK4 / Android NDK) at link
// time — the same composition code drives every target.

#include <mpapp/run.hpp>

#include "uiss/app.hpp"

int main(int argc, char** argv) {
    return mpapp::run<uiss::uiss_app>(argc, argv);
}
