set(MAIN_SRCS ${SRCS} main.cpp)

# Include gtest_prod directly so we don't have to link against the entire gtest lib for production
# executables
set(GTEST_INCLUDE_PATH ../build/_deps/googletest-src/googletest/include/)

# Main switch production executable with all the checks on top
add_executable(switch ${MAIN_SRCS})
add_dependencies(switch clang-tidy unit)
target_include_directories(switch PRIVATE ${GTEST_INCLUDE_PATH})

# Production-like executable without static analysis or unit tests
add_executable(dev-switch ${MAIN_SRCS})
target_include_directories(dev-switch PRIVATE ${GTEST_INCLUDE_PATH})

# Debug executable
add_executable(debug-switch ${MAIN_SRCS})
target_compile_options(debug-switch PRIVATE -g -fsanitize=address)
target_link_options(debug-switch PRIVATE -fsanitize=address)

# clang-tidy
add_custom_target(clang-tidy
    COMMAND
        clang-tidy *.cpp *.hpp -- ${CXX_STANDARD_FLAG} -I${GTEST_INCLUDE_PATH}
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}
)

# clang-format
add_custom_target(clang-format
    COMMAND
        clang-format -i *.cpp *.hpp
    WORKING_DIRECTORY
        ${CMAKE_CURRENT_LIST_DIR}
)

