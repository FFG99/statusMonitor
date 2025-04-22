#include "metrics/MemoryMetric.hpp"
#include <gtest/gtest.h>
#include <variant>

TEST(MemoryMetricTest, ValidConfig) {
    json config = {{"spec", {"MemTotal", "MemFree"}}};
    MemoryMetric metric(config);
    EXPECT_TRUE(metric.is_valid());
}

TEST(MemoryMetricTest, InvalidConfigMissingField) {
    json config = {{"wrong_field", {"MemTotal"}}};
    MemoryMetric metric(config);
    EXPECT_FALSE(metric.is_valid());
}

TEST(MemoryMetricTest, InvalidConfigEmptyArray) {
    json config = {{"spec", json::array()}};
    MemoryMetric metric(config);
    EXPECT_FALSE(metric.is_valid());
}

TEST(MemoryMetricTest, Name) {
    json config = {{"spec", {"MemTotal"}}};
    MemoryMetric metric(config);
    EXPECT_EQ(metric.name(), "memory");
}

TEST(MemoryMetricTest, Collect) {
    json config = {{"spec", {"MemTotal", "MemFree"}}};
    MemoryMetric metric(config);
    ASSERT_TRUE(metric.is_valid());

    auto result = metric.collect();

    bool is_map = std::holds_alternative<std::map<std::string, double>>(result);
    EXPECT_TRUE(is_map);

    if (is_map) {
        auto memory = std::get<std::map<std::string, double>>(result);
        EXPECT_FALSE(memory.empty());
        EXPECT_GT(memory["MemTotal"], 0.0);
    }
}

TEST(MemoryMetricTest, InvalidMetricNames) {
    json config = {{"spec", {"NonExistentMetric", "MemTotal"}}};
    MemoryMetric metric(config);
    ASSERT_TRUE(metric.is_valid());
    
    auto result = metric.collect();
    bool is_map = std::holds_alternative<std::map<std::string, double>>(result);
    EXPECT_TRUE(is_map);
    
    if (is_map) {
        auto memory = std::get<std::map<std::string, double>>(result);
        EXPECT_TRUE(memory.find("MemTotal") != memory.end());
        EXPECT_TRUE(memory.find("NonExistentMetric") == memory.end());
    }
}

TEST(MemoryMetricTest, EmptyMetricNames) {
    json config = {{"spec", {""}}};
    MemoryMetric metric(config);
    EXPECT_FALSE(metric.is_valid());
}

TEST(MemoryMetricTest, DuplicateMetricNames) {
    json config = {{"spec", {"MemTotal", "MemTotal"}}};
    MemoryMetric metric(config);
    EXPECT_FALSE(metric.is_valid());
}

TEST(MemoryMetricTest, MemoryValuesRange) {
    json config = {{"spec", {"MemTotal", "MemFree", "MemAvailable"}}};
    MemoryMetric metric(config);
    ASSERT_TRUE(metric.is_valid());
    
    auto result = metric.collect();
    bool is_map = std::holds_alternative<std::map<std::string, double>>(result);
    EXPECT_TRUE(is_map);
    
    if (is_map) {
        auto memory = std::get<std::map<std::string, double>>(result);
        EXPECT_GT(memory["MemTotal"], 0.0);
        EXPECT_GE(memory["MemFree"], 0.0);
        EXPECT_GE(memory["MemAvailable"], 0.0);
        EXPECT_LE(memory["MemFree"], memory["MemTotal"]);
        EXPECT_LE(memory["MemAvailable"], memory["MemTotal"]);
    }
}