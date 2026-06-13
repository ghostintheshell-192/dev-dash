cmake_minimum_required(VERSION 3.28)

project("dev-dash" LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME} not supported yet (Linux only)")
endif()

# ----- Options -----

option(DEVDASH_BUILD_TESTS "Build the dev-dash unit tests (Catch2)" ON)

# ----- CPM -----

include(cmake/get_cpm.cmake)

# ----- Subprojects -----

add_subdirectory(external)
add_subdirectory(src)

if(DEVDASH_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
