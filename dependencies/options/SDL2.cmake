# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# Design/direction by Togorian; implementation with Codex assistance.
# Those contributions are offered under GPL-2.0-or-later; upstream notices remain.
# Modified 11 November 2026: provenance/notice clarification only. See PROVENANCE.md.
# Initial cache options for the preserved source; ordinary developer overrides remain possible.
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW CACHE STRING "Use explicit MSVC runtime selection")
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded CACHE STRING "Static MSVC runtime")
set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "Selected compatible-build option")
set(SDL_INSTALL_TESTS OFF CACHE BOOL "Selected compatible-build option")
set(SDL_SHARED OFF CACHE BOOL "Selected compatible-build option")
set(SDL_STATIC ON CACHE BOOL "Selected compatible-build option")
set(SDL_TEST OFF CACHE BOOL "Selected compatible-build option")
set(SDL_TESTS OFF CACHE BOOL "Selected compatible-build option")

# Keep desktop OpenGL and Vulkan; exclude OpenGL ES and EGL from this selection.
set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES and EGL")
set(SDL_OPENGL ON CACHE BOOL "Retain desktop OpenGL")
set(SDL_VULKAN ON CACHE BOOL "Retain Vulkan")
