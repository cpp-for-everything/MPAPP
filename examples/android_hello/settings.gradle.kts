// SPDX-License-Identifier: Apache-2.0
// Part of MPAPP. T-0011 — Android hello example Gradle settings.

pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
    }
}

rootProject.name = "android_hello"
include(":app")
