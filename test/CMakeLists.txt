add_executable(logmore_unit_tests
        parser_test.cpp
        )
target_link_libraries(logmore_unit_tests
        gtest_main
        logmore::lib
        logmore::compile_options
        )

include(GoogleTest)
gtest_discover_tests(logmore_unit_tests)