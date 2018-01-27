cmake_minimum_required(VERSION 3.8)
project(hrlib)

include_directories(./include)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.62.0 REQUIRED
             COMPONENTS unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

enable_testing()
add_library(
        boost_unit_test_framework_static
        STATIC
        IMPORTED
)
set_target_properties(
        boost_unit_test_framework_static
        PROPERTIES
        IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/libboost_unit_test_framework.a
)
add_subdirectory(test/hrlib)

