add_library(logmore_lib
        STATIC
        logmore/parser.cpp
        logmore/parser.hpp
        logmore/types.hpp
        )
add_library(logmore::lib ALIAS logmore_lib)
target_link_libraries(logmore_lib
        PUBLIC
        Boost::pfr
        fmt::fmt

        PRIVATE
        logmore::compile_options
        )
target_include_directories(logmore_lib
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )