add_executable(logmore
        main.cpp
        )

target_link_libraries(logmore
        PUBLIC
        logmore::lib

        PRIVATE
        fire::project_options
        )
