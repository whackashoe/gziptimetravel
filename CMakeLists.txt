cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project (gziptimetravel)

set(GZIPTIMETRAVEL_VERSION_MAJOR 0)
set(GZIPTIMETRAVEL_VERSION_MINOR 1)

configure_file (
    "${PROJECT_SOURCE_DIR}/gziptimetravel.h.in"
    "${PROJECT_SOURCE_DIR}/gziptimetravel.h"
)

include_directories ("${PROJECT_BINARY_DIR}")

add_executable (gziptimetravel gziptimetravel.c)

install (TARGETS gziptimetravel DESTINATION bin)
