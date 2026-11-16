# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# SPDX-License-Identifier: GPL-2.0-or-later
# Project release recipe, Togorian with Codex assistance, rights held GPLv2-or-later.
option(Q2RTX_RELEASE_RECORD "Expose the explicit release receipt target" OFF)
if(Q2RTX_RELEASE_RECORD)
  find_package(Python3 3.10 COMPONENTS Interpreter REQUIRED)
  set(Q2RTX_SHADER_BUILD "${CMAKE_BINARY_DIR}/shader-output" CACHE PATH "Fresh verified shader build for the receipt")
  # Build all products first, retain completed logs/context, then record.
  add_custom_target(q2rtx_release_products DEPENDS q2rtx gamex86_64 Q2RTXVRLauncher)
  add_custom_target(q2rtx_release_record
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/packaging/package.py"
      record --source "${CMAKE_CURRENT_SOURCE_DIR}" --build "${CMAKE_BINARY_DIR}"
      --libraries "${Q2RTX_DEPENDENCIES}" --shaders "${Q2RTX_SHADER_BUILD}"
    DEPENDS q2rtx gamex86_64 Q2RTXVRLauncher
    VERBATIM)
endif()
