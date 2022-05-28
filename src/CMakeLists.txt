add_executable(logmore
        main.cpp
        )

target_link_libraries(logmore
        PUBLIC
        logmore::lib

        PRIVATE
        logmore::compile_options
        )
