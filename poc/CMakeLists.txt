cmake_minimum_required(VERSION 3.28)

project("dev-dash-poc" LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ninja generator has a bug with paths containing spaces during C++ module dependency scanning.
# Modules are unused here; turn the scan off to avoid the issue regardless.
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ----- Platform / compiler dispatch -----

if(NOT "${CMAKE_SIZEOF_VOID_P}" MATCHES 8)
    message(FATAL_ERROR "32-bit builds are not supported")
endif()

if(LINUX)
    if(CMAKE_C_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        include(cmake/compilers/gcc.cmake)
    else()
        message(FATAL_ERROR "Only GCC is supported on Linux for now (C=${CMAKE_C_COMPILER_ID}, CXX=${CMAKE_CXX_COMPILER_ID})")
    endif()
else()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} not supported in the PoC yet (Linux only)")
endif()

# ----- CPM (package manager for source-from-GitHub deps) -----

include(cmake/get_cpm.cmake)

# ----- Subprojects -----

add_subdirectory(external)
add_subdirectory(src)
