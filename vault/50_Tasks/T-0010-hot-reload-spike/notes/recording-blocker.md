---
type: log
area: tooling
tags:
  - type/log
  - area/tooling
  - platform/windows
---

# T-0010 — recording blocker

The worker environment in which this spike was implemented does not have:

1. A GUI / desktop session capable of running a screen-recorder.
2. `clang++` on `PATH` (only the Visual Studio 18 / MSVC toolchain is
   available). The runtime invokes `clang++ -std=c++23 -shared`, so the
   rebuild step cannot be executed end-to-end here.

What was verified end-to-end inside the worker:

- `cmake -S . -B build -G Ninja` configures cleanly with the new
  `MPAPP_BUILD_EXAMPLES` option.
- `cmake --build build` compiles `mpapp-core` with the new
  `src/hot_reload/windows.cpp`, links `hot_reload_spike_host.exe`,
  and stages `user_code.cpp` / `user_code.h` next to the host.
- `ctest --output-on-failure` runs 14/14 tests, including:
  - `Hot<T> is an empty tag base`
  - `runtime fails cleanly when source is missing`
- Public-API macro check (`grep -rn 'MPAPP_[A-Z_]*(' include/mpapp/`)
  returns no matches.

The two Catch test cases that exercise the full clang rebuild + swap
loop carry the `[.clang]` tag so they are excluded from the default
ctest run when clang isn't on PATH. They skip cleanly via `SKIP(...)`
when invoked directly.

## Action for the next workstation that has clang + a GUI

1. Install LLVM 18 (or later) and put `clang++` on PATH:

   ```bat
   clang++ --version
   ```

2. Re-run the build:

   ```bat
   _build_t0010.bat
   ```

3. Run the host:

   ```bat
   cd build\examples\hot_reload_spike
   hot_reload_spike_host.exe
   ```

4. Edit `user_code.cpp` (the staged copy in the same directory), change
   `return x * 2;` to `return x * 10;`, save.
5. Capture the screen recording as `swap.mp4` and replace
   `vault/50_Tasks/T-0010-hot-reload-spike/recordings/.gitkeep` with it.
6. Set `hasRecordings: true` in the task frontmatter (already done by
   this task; the file just becomes real).
