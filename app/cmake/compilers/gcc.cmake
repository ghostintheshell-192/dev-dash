# GCC-specific settings.

# Reject GCC versions older than 12 (the lowest Germen supports too).
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
    message(FATAL_ERROR "GCC ${CMAKE_CXX_COMPILER_VERSION} is too old; need >= 12")
endif()

message(STATUS "GCC: enabling position-independent code (-fPIC)")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Warning flags carried by an INTERFACE target rather than CMAKE_*_FLAGS, so
# they apply ONLY to our code — not to the CPM dependencies (ImGui, SDL, md4c)
# built in the same tree, which would otherwise drown us in third-party noise.
# Deliberately NOT -Werror locally, to keep development fluid; CI promotes
# these to errors (DEVDASH_WERROR=ON, set by the linux-ci preset) so nothing
# slips through the pipeline.
option(DEVDASH_WERROR "Treat compiler warnings as errors (CI)" OFF)

add_library(dev-dash-warnings INTERFACE)
target_compile_options(dev-dash-warnings INTERFACE -Wall -Wextra)
if(DEVDASH_WERROR)
    target_compile_options(dev-dash-warnings INTERFACE -Werror)
endif()
