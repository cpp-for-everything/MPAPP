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
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "0.1.0"

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DCMAKE_CXX_STANDARD=23",
                    "-DCMAKE_CXX_SCAN_FOR_MODULES=OFF"
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
