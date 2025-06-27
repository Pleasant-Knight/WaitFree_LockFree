# CMakeLists.txt for the tests directory
add_executable(wcq_tests wCQ_tests.cpp)
target_include_directories(wcq_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(wcq_tests PRIVATE wfq GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(wcq_tests)

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark PRIVATE wfq)
target_include_directories(benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_features(benchmark PRIVATE cxx_std_17)
set_target_properties(benchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
