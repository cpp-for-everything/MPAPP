# T-0010 Hot-reload spike (Windows desktop)

A minimal demo of the C++ hot-reload pattern described in
[`vault/10_Architecture/Hot Reload.md`](../../vault/10_Architecture/Hot%20Reload.md):

1. The host program (`hot_reload_spike_host.exe`) loads `user_code.dll`.
2. The runtime polls `user_code.cpp`'s mtime once per second.
3. When the source changes, the runtime invokes `clang++ -shared` to rebuild
   the dll, calls `FreeLibrary` on the old image, and `LoadLibraryEx`'s the
   new one.
4. The host continues running with the new behavior.

## Prerequisites

- Windows (the runtime is gated on `WIN32`).
- `clang++` on PATH. Test with `clang++ --version`.
- CMake 3.28+, Ninja (or any generator), an MSVC environment for building
  the host itself.

## Build

```bat
cmake -S . -B build -G Ninja -DMPAPP_BUILD_EXAMPLES=ON
cmake --build build --target hot_reload_spike_host
```

`user_code.cpp` and `user_code.h` are copied next to the resulting
`hot_reload_spike_host.exe` so the host can find them with a relative
working directory.

## Manual demo

1. Open a command prompt and `cd` to the directory containing
   `hot_reload_spike_host.exe` (typically `build/examples/hot_reload_spike`).
2. Run `hot_reload_spike_host.exe`. You should see:

   ```
   [host] watching user_code.cpp
   [host] edit user_code.cpp and save to trigger reload
   compute(1) = 2
   compute(2) = 4
   compute(3) = 6
   ...
   ```

3. Open `user_code.cpp` in your editor (the copy next to the exe). Change
   `return x * 2;` to `return x * 10;`. Save.
4. Within ~1 second the host prints:

   ```
   [reloaded]
   compute(N) = 10*N
   ```

5. Repeat the edit-save cycle with any expression of your choosing.

## Recording

Capture the entire edit -> save -> [reloaded] -> new-output sequence as a
screen recording. Save to
`vault/50_Tasks/T-0010-hot-reload-spike/recordings/swap.mp4` (or `.gif`).

If recording is not possible in the current environment, write a text log
of the same sequence to
`vault/50_Tasks/T-0010-hot-reload-spike/notes/swap-log.txt` and a blocker
note at `notes/recording-blocker.md`.

## What this demonstrates

- The `LoadLibraryEx` / `FreeLibrary` + Clang/LLD dll-rebuild loop works
  on Windows desktop.
- `mpapp::Hot<T>` (an empty tag base today) is the future opt-in for
  state-preserving hot reload (full mechanism is out of scope for this
  spike).
- The dev loop pattern Android emulator and iOS Simulator hot reload
  will build on later.
