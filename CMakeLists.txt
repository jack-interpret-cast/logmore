cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(src LANGUAGES CXX)

include(cmake/project_settings.cmake)
common_setup()

# Setup Externals
FetchContent_Declare(boost_pfr
        GIT_REPOSITORY https://github.com/boostorg/pfr.git
        GIT_TAG 2.0.3
        GIT_SHALLOW ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        )
FetchContent_MakeAvailable(boost_pfr)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        GIT_SHALLOW ON
        )
FetchContent_MakeAvailable(fmt)

FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
        GIT_SHALLOW ON
        )
FetchContent_MakeAvailable(googletest)

set(CMAKE_CURSES_NEED_WIDE true)
find_package(Curses REQUIRED)

find_package(Boost REQUIRED COMPONENTS System)

add_subdirectory(include)
add_subdirectory(src)
enable_testing()
add_subdirectory(test)

