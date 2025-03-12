cmake_minimum_required(VERSION 3.14)
project(ShellProject)

# Set the compipler to the appoproateone
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  DOWNLOAD_EXTRACT_TIMESTAMP false
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
FetchContent_MakeAvailable(googletest)

enable_testing()

include_directories(include/)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(shell src/shell.c)

add_executable(shell_test gtest/shell_test.cpp)

target_link_libraries(shell_test GTest::gtest_main)

include(GoogleTest)

gtest_discover_tests(shell_test)
