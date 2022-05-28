enable_testing()

add_executable(hello_test
        hello_test.cpp
        )
target_link_libraries(hello_test
        gtest_main
        logmore::lib
        logmore::compile_options
        )

include(GoogleTest)
gtest_discover_tests(hello_test)