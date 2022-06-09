add_library(logmore_lib
        STATIC
        logmore/file_handler.cpp
        logmore/file_handler.hpp
        logmore/terminal.cpp
        logmore/terminal.hpp
        logmore/keys.hpp
        logmore/config.hpp
        logmore/controller.cpp
        logmore/controller.hpp
        logmore/application.cpp
        logmore/application.hpp
        )
add_library(logmore::lib ALIAS logmore_lib)

target_link_libraries(logmore_lib
        PUBLIC
        Boost::pfr
        fmt::fmt
        ${CURSES_LIBRARIES}
        magic_enum::magic_enum
        fire::fire

        PRIVATE
        fire::project_options
        )
target_include_directories(logmore_lib
        INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
        )