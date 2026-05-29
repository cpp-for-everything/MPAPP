# T-0050 tests folder

The canonical Catch2 test for RFC-0006 lives at:

```
tests/mock_handlers/visual_state_manager_test.cpp
```

It is auto-picked-up by the CTest glob in `tests/CMakeLists.txt`, so every CI run builds + executes it. 8 cases, 36 assertions. Total ctest count went from 395 → 403 green when this RFC landed.

`android-headers-smoke.cpp` (in this folder) is the cross-compile smoke source — compile it under any Android NDK clang to verify the new headers stay clean on Android:

```
$ NDK=$HOME/Android/Sdk/ndk/26.1.10909125/toolchains/llvm/prebuilt/linux-x86_64
$ "$NDK/bin/aarch64-linux-android28-clang++" \
     -std=c++2b -Iinclude -Wall -Wextra -Wpedantic \
     -c vault/50_Tasks/T-0050-rfc-0006-vsm-mock/tests/android-headers-smoke.cpp \
     -o /tmp/smoke.o
```

Two .o files produced for the Windows-host NDK r26 run (paths under `build/`):

```
build/android-headers-smoke-aarch64.o  (≈625 KB)
build/android-headers-smoke-x86_64.o   (≈586 KB)
```
