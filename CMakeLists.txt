cmake_minimum_required(VERSION 3.10)
project(statusMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# FetchContent для GTest и nlohmann/json
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(googletest json)

set(SOURCES
    src/metrics/CPUMetric.cpp
    src/metrics/MemoryMetric.cpp
    src/output/ConsoleOutput.cpp
    src/output/FileOutput.cpp
    src/main.cpp
)

add_library(metrics STATIC 
    src/metrics/CPUMetric.cpp
    src/metrics/MemoryMetric.cpp
    src/output/ConsoleOutput.cpp
    src/output/FileOutput.cpp
)
target_include_directories(metrics PUBLIC include)
target_link_libraries(metrics PUBLIC nlohmann_json::nlohmann_json)

add_executable(status_monitor src/main.cpp)
target_link_libraries(status_monitor
    metrics
    nlohmann_json::nlohmann_json
)

# Тесты для метрик
set(METRICS_TEST_SOURCES
    tests/metrics/CPUMetricTest.cpp
    tests/metrics/MemoryMetricTest.cpp
)

add_executable(metrics_test ${METRICS_TEST_SOURCES})
target_link_libraries(metrics_test
    metrics
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME metrics_test COMMAND metrics_test)

# Тесты для выводов
set(OUTPUTS_TEST_SOURCES
    tests/output/ConsoleOutputTest.cpp
    tests/output/FileOutputTest.cpp
)

add_executable(outputs_test ${OUTPUTS_TEST_SOURCES})
target_link_libraries(outputs_test
    metrics
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME outputs_test COMMAND outputs_test) 