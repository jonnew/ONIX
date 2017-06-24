cmake_minimum_required (VERSION 2.8)

# Project data
project (liboepcie C CXX)

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -Wextra -Wpedantic -Wno-mismatched-tags -Wno-unused-parameter") #
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math -fPIC -fpic")

# Make sure a build type has been specified
if (NOT CMAKE_BUILD_TYPE)
   set (CMAKE_BUILD_TYPE Release ... FORCE )
endif ()

# Build type case insenstivity
string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASE_CMAKE_BUILD_TYPE)

# Switch compiler flags based on build type
if (LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "debug")
    set (FULL_CXX_FLAG_LIST "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif (LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "release")
    set (FULL_CXX_FLAG_LIST "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    set (BUILD_TESTS OFF)
else ()
    message (STATUS "No build type specified using -DCMAKE_BUILD_TYPE=<debug or release>.")
    message (STATUS "Defaulting to release.")
    set (LOWERCASE_CMAKE_BUILD_TYPE "release")
    set (CMAKE_BUILD_TYPE release)
    set (FULL_CXX_FLAG_LIST "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    set (BUILD_TESTS OFF)
endif ()

# Local includes
include_directories ("${PROJECT_SOURCE_DIR}/include")

# Linker options
#set_target_properties (oepcie PROPERTIES LINKER_LANGUAGE CXX)

# Examples
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/examples/serdes-128)
