# Copyright (C) 2026 Togorian, to the extent of rights held in project additions.
# SPDX-License-Identifier: GPL-2.0-or-later
# Project build glue, Togorian with Codex assistance; rights held offered under GPLv2-or-later.
# Modified 11 November 2026: one product version definition, separate origin.
get_filename_component(Q2RTX_SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
file(READ "${Q2RTX_SOURCE_ROOT}/VERSION" Q2RTX_PRODUCT_VERSION)
string(STRIP "${Q2RTX_PRODUCT_VERSION}" Q2RTX_PRODUCT_VERSION)
if(NOT Q2RTX_PRODUCT_VERSION MATCHES "^(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)\\.(0|[1-9][0-9]*)(-rc\\.([1-9][0-9]*))?$")
  message(FATAL_ERROR "VERSION must contain major.minor.patch or major.minor.patch-rc.number")
endif()
set(Q2RTX_MAJOR "${CMAKE_MATCH_1}")
set(Q2RTX_MINOR "${CMAKE_MATCH_2}")
set(Q2RTX_PATCH "${CMAKE_MATCH_3}")
set(Q2RTX_RC "${CMAKE_MATCH_5}")
set(Q2RTX_PRERELEASE 2)
if(Q2RTX_RC STREQUAL "")
  set(Q2RTX_RC 0)
  set(Q2RTX_PRERELEASE 0)
endif()
foreach(part Q2RTX_MAJOR Q2RTX_MINOR Q2RTX_PATCH Q2RTX_RC)
  if(${part} GREATER 65535)
    message(FATAL_ERROR "VERSION resource components must be <=65535")
  endif()
endforeach()
if(NOT DEFINED Q2RTX_VERSION_OUTPUT)
  set(Q2RTX_VERSION_OUTPUT "${CMAKE_BINARY_DIR}/product-version")
endif()
file(MAKE_DIRECTORY "${Q2RTX_VERSION_OUTPUT}")
configure_file("${CMAKE_CURRENT_LIST_DIR}/product-version.h.in"
  "${Q2RTX_VERSION_OUTPUT}/q2rtx-product-version.h" @ONLY)
configure_file("${CMAKE_CURRENT_LIST_DIR}/upstream-origin.h"
  "${Q2RTX_VERSION_OUTPUT}/q2rtx-upstream-origin.h" COPYONLY)
