cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(logless)

include(${CMAKE_SOURCE_DIR}/cmake/project_settings.cmake)
common_setup()

# Setup Externals
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED True)
FetchContent_Declare(boost_pfr
        GIT_REPOSITORY https://github.com/boostorg/pfr.git
        GIT_TAG 2.0.3
        GIT_SHALLOW ON
        GIT_PROGRESS ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        )
FetchContent_MakeAvailable(boost_pfr)

add_executable(logless
        main.cpp
        )
target_compile_options(logless
        PRIVATE -Wall -Wextra -Werror
        )