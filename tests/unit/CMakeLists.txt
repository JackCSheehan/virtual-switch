# Google Test dependency
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/b514bdc898e2951020cbdca1304b75f5950d1f59.zip
)

FetchContent_MakeAvailable(googletest)

file(GLOB UNIT_SRCS *.cpp)

add_executable(test-switch ${UNIT_SRCS} ${SRCS})
target_link_libraries(test-switch GTest::gtest_main gmock_main)
target_include_directories(test-switch PRIVATE ../../src)

include(GoogleTest)
gtest_discover_tests(test-switch)

add_custom_target(unit COMMAND ctest --output-on-failure)
add_dependencies(unit test-switch)

