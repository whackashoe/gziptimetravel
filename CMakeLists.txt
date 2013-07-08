cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project (gziptimetravel)

set(CMAKE_BUILD_TYPE DEBUG)
set(CMAKE_C_FLAGS "-pedantic -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")

set(GZIPTIMETRAVEL_VERSION_MAJOR 0)
set(GZIPTIMETRAVEL_VERSION_MINOR 2)

configure_file (
    "${PROJECT_SOURCE_DIR}/gziptimetravel.h.in"
    "${PROJECT_SOURCE_DIR}/gziptimetravel.h"
)

include_directories ("${PROJECT_BINARY_DIR}")

add_executable (gziptimetravel gziptimetravel.c)

install (TARGETS gziptimetravel DESTINATION bin)
