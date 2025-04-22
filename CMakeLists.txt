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

# Создаем shared library для каждой метрики
add_library(cpu_metric SHARED
    src/metrics/CPUMetric.cpp
)
target_include_directories(cpu_metric PUBLIC include)
target_link_libraries(cpu_metric PUBLIC nlohmann_json::nlohmann_json)
set_target_properties(cpu_metric PROPERTIES
    PREFIX ""
    OUTPUT_NAME "cpu_metric"
)

add_library(memory_metric SHARED
    src/metrics/MemoryMetric.cpp
)
target_include_directories(memory_metric PUBLIC include)
target_link_libraries(memory_metric PUBLIC nlohmann_json::nlohmann_json)
set_target_properties(memory_metric PROPERTIES
    PREFIX ""
    OUTPUT_NAME "memory_metric"
)

# Основное приложение
add_executable(status_monitor 
    src/main.cpp
    src/output/ConsoleOutput.cpp
    src/output/FileOutput.cpp
)
target_include_directories(status_monitor PUBLIC include)
target_link_libraries(status_monitor
    nlohmann_json::nlohmann_json
    dl
)

# Тесты для метрик
set(METRICS_TEST_SOURCES
    tests/metrics/CPUMetricTest.cpp
    tests/metrics/MemoryMetricTest.cpp
)

add_executable(metrics_test ${METRICS_TEST_SOURCES})
target_link_libraries(metrics_test
    cpu_metric
    memory_metric
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME metrics_test COMMAND metrics_test)

# Тесты для выходов
set(OUTPUTS_TEST_SOURCES
    tests/output/ConsoleOutputTest.cpp
    tests/output/FileOutputTest.cpp
    src/output/ConsoleOutput.cpp
    src/output/FileOutput.cpp
)

add_executable(outputs_test ${OUTPUTS_TEST_SOURCES})
target_link_libraries(outputs_test
    cpu_metric
    memory_metric
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME outputs_test COMMAND outputs_test)
