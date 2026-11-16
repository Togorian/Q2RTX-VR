# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
# Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
cmake_minimum_required(VERSION 3.25)
set(CMAKE_TRY_COMPILE_CONFIGURATION Release)
project(Q2RTXVRLauncherChecks LANGUAGES C)
if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "These checks require Windows x64 MSVC.")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/../../recipes/product-version.cmake")
include_directories("${Q2RTX_VERSION_OUTPUT}")
add_executable(launcher_checks checks.c)
target_compile_options(launcher_checks PRIVATE /W4 /std:c11 /utf-8)
target_link_libraries(launcher_checks PRIVATE shell32.lib kernel32.lib)
enable_testing()
add_test(NAME launcher_cpu_checks COMMAND launcher_checks)
