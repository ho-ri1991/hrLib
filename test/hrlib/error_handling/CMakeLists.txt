cmake_minimum_required(VERSION 3.8)

add_executable(result_test result.cpp)
target_link_libraries(result_test boost_unit_test_framework_static)
add_test(
        NAME result_test
        COMMAND $<TARGET_FILE:result_test>
)
set_property(TEST result_test PROPERTY LABELS result_test)

