add_executable(logmore_unit_tests
        controller_test.cpp
        )
target_link_libraries(logmore_unit_tests
        gtest_main
        logmore::lib
        fire::project_options
        )

include(GoogleTest)
gtest_discover_tests(logmore_unit_tests)