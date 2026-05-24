// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android hello example app module.

plugins {
    id("com.android.application")
}

android {
    namespace = "io.mpapp.example"
    compileSdk = 34
    ndkVersion = "26.1.10909125"

    defaultConfig {
        applicationId = "io.mpapp.example"
        // minSdk = 28 because the Cairo backend (ADR-0015) depends on
        // fontconfig, which uses iconv — Android's bionic libiconv
        // landed in API 28. Apps that don't need real Cairo rendering
        // can drop the MPAPP_GRAPHICS_BACKEND=cairo CMake arg (in the
        // externalNativeBuild block below) to get the stub backend
        // and re-target minSdk 24.
        minSdk = 28
        targetSdk = 34
        versionCode = 1
        versionName = "0.1.0"

        externalNativeBuild {
            cmake {
                // Resolve vcpkg paths up front so the cmake arguments below
                // can reference them without inline interpolation. Both env
                // vars are optional: if VCPKG_ROOT is unset the Cairo
                // backend simply falls back to stub (see
                // src/main/cpp/CMakeLists.txt detection logic).
                //
                // MPAPP_GRAPHICS_BACKEND can be overridden via
                // -PmpappGraphicsBackend=skia at gradle invocation time
                // (or by editing the default below). Default is cairo —
                // matches the host build's default and the M-04c rollout
                // baseline.
                //
                // Skia (when selected) defaults to the pinned HumbleUI
                // prebuilt auto-downloaded by `cmake/MpappFindSkia.cmake`
                // via FetchContent. No vcpkg required. To override with
                // a vcpkg installed/<triplet>-android dir or a manually
                // unzipped HumbleUI/JetBrains drop, pass
                // `-PmpappSkiaPrefix=<dir>` — the property is forwarded
                // to CMake as `-DMPAPP_SKIA_PREFIX=...` only when set.
                val vcpkgRoot = System.getenv("VCPKG_ROOT")
                    ?: "${System.getProperty("user.home").replace('\\', '/')}/vcpkg"
                val triplet     = "x64-android"  // matches abiFilter below
                val vcpkgPrefix = "$vcpkgRoot/installed/$triplet"
                val pkgconfBin  = "$vcpkgRoot/downloads/tools/msys2/3e71d1f8e22ab23f/mingw64/bin/pkgconf.exe"
                val backend     = (project.findProperty("mpappGraphicsBackend") as String?)
                                    ?: "cairo"
                val skiaPrefix  = (project.findProperty("mpappSkiaPrefix") as String?)
                                    ?.replace('\\', '/')

                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DCMAKE_CXX_STANDARD=23",
                    "-DCMAKE_CXX_SCAN_FOR_MODULES=OFF",
                    "-DMPAPP_GRAPHICS_BACKEND=$backend",
                    "-DMPAPP_CAIRO_PREFIX=$vcpkgPrefix",
                    "-DCMAKE_PREFIX_PATH=$vcpkgPrefix",
                    "-DPKG_CONFIG_EXECUTABLE=$pkgconfBin"
                )
                if (skiaPrefix != null) {
                    arguments += "-DMPAPP_SKIA_PREFIX=$skiaPrefix"
                }
                cppFlags += listOf("-std=c++23")
            }
            ndk {
                abiFilters += listOf("x86_64")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.28.0+"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    buildTypes {
        debug {
            isMinifyEnabled = false
        }
        release {
            isMinifyEnabled = false
        }
    }
}

dependencies {
    // T-0028: AndroidX RecyclerView powers the collection_view real
    // handler's four-layout matrix (vertical/horizontal × list/grid).
    // The legacy AbsListView-based widgets (ListView, GridView) are
    // vertical-only; RecyclerView + LinearLayoutManager/GridLayoutManager
    // honor an orientation arg so the same widget covers all four
    // layouts via a setLayoutManager swap.
    implementation("androidx.recyclerview:recyclerview:1.3.2")
}
