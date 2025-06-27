cmake_minimum_required(VERSION 3.14)
project(wait_free_cpp CXX)

add_subdirectory(libcds)
add_subdirectory(src)
add_executable(demo src/main.cpp)
target_include_directories(demo PRIVATE src)
target_link_libraries(demo PRIVATE cds)
target_compile_features(demo PRIVATE cxx_std_17)

# Enable C++17 standard
set_target_properties(demo PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(demo PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(demo PRIVATE /W4 /permissive-)
endif()
# Enable optimizations
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(demo PRIVATE -O2)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(demo PRIVATE -O0 -g)
endif()


include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)

# Manually populate and add
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})
endif()

enable_testing()
add_subdirectory(tests)