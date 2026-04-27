# GCC-specific settings.

# Reject GCC versions older than 12 (the lowest Germen supports too).
if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
    message(FATAL_ERROR "GCC ${CMAKE_CXX_COMPILER_VERSION} is too old; need >= 12")
endif()

message(STATUS "GCC: enabling position-independent code (-fPIC)")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
