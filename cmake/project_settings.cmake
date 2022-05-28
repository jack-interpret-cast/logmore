macro(common_setup)
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    # For external tooling
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    # Faster compilation
    find_program(CCACHE ccache)
    if (${CCACHE} STREQUAL "CCACHE-NOTFOUND")
        message(WARNING "ccache not found")
    else ()
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
    endif ()

    # Faster linking
    find_program(ALTERNATIVE_LINKER ld.mold)
    if (${ALTERNATIVE_LINKER} STREQUAL "ALTERNATIVE_LINKER-NOTFOUND")
        message(WARNING "mold linker not found")
    else ()
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
            add_link_options("-B/usr/lib/mold")
        else ()
            add_link_options("-fuse-ld=${ALTERNATIVE_LINKER}")
        endif ()
    endif ()

    # Common lib/exe properties
    add_library(project_options INTERFACE)
    add_library(logmore::compile_options ALIAS project_options)
    target_compile_features(project_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
    target_compile_options(project_options
            INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Werror
            )
    include(FetchContent)
    set(FETCHCONTENT_UPDATES_DISCONNECTED True)
    set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/externals)
endmacro()

