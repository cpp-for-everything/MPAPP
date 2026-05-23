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
                val vcpkgRoot = System.getenv("VCPKG_ROOT")
                    ?: "${System.getProperty("user.home").replace('\\', '/')}/vcpkg"
                val cairoTriplet = "x64-android"  // matches abiFilter below
                val cairoPrefix  = "$vcpkgRoot/installed/$cairoTriplet"
                val pkgconfBin   = "$vcpkgRoot/downloads/tools/msys2/3e71d1f8e22ab23f/mingw64/bin/pkgconf.exe"

                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DCMAKE_CXX_STANDARD=23",
                    "-DCMAKE_CXX_SCAN_FOR_MODULES=OFF",
                    "-DMPAPP_GRAPHICS_BACKEND=cairo",
                    "-DMPAPP_CAIRO_PREFIX=$cairoPrefix",
                    "-DCMAKE_PREFIX_PATH=$cairoPrefix",
                    "-DPKG_CONFIG_EXECUTABLE=$pkgconfBin"
                )
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
