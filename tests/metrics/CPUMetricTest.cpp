#include "metrics/CPUMetric.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <variant>

TEST(CPUMetricTest, ValidConfig) {
    json config = {{"cpu_ids", {0, 1, 2}}};
    CPUMetric metric(config);
    EXPECT_TRUE(metric.is_valid());
}

TEST(CPUMetricTest, InvalidConfigMissingField) {
    json config = {{"wrong_field", {0, 1}}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, InvalidConfigEmptyArray) {
    json config = {{"cpu_ids", json::array()}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, InvalidConfigWrongType) {
    json config = {{"cpu_ids", {0, "1", 2}}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, Name) {
    json config = {{"cpu_ids", {0}}};
    CPUMetric metric(config);
    EXPECT_EQ(metric.name(), "cpu");
}

TEST(CPUMetricTest, CollectSingleCPU) {
    json config = {{"cpu_ids", {0}}};
    CPUMetric metric(config);
    ASSERT_TRUE(metric.is_valid());

    auto result = metric.collect();

    bool is_vector = std::holds_alternative<std::vector<double>>(result);
    EXPECT_TRUE(is_vector);

    if (is_vector) {
        auto usage = std::get<std::vector<double>>(result);
        EXPECT_EQ(usage.size(), 1);
        EXPECT_GE(usage[0], 0.0);
        EXPECT_LE(usage[0], 100.0);
    }
}

TEST(CPUMetricTest, CollectMultipleCPUs) {
    json config = {{"cpu_ids", {0, 1}}};
    CPUMetric metric(config);
    ASSERT_TRUE(metric.is_valid());

    auto result = metric.collect();

    bool is_vector = std::holds_alternative<std::vector<double>>(result);
    EXPECT_TRUE(is_vector);

    if (is_vector) {
        auto usage = std::get<std::vector<double>>(result);
        EXPECT_EQ(usage.size(), 2);
        for (const auto &value : usage) {
            EXPECT_GE(value, 0.0);
            EXPECT_LE(value, 100.0);
        }
    }
}

TEST(CPUMetricTest, InvalidConfigNegativeCPU) {
    json config = {{"cpu_ids", {-1, 0, 1}}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, InvalidConfigDuplicateCPU) {
    json config = {{"cpu_ids", {0, 0, 1}}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, InvalidConfigNonNumeric) {
    json config = {{"cpu_ids", {0, "invalid", 1}}};
    EXPECT_THROW(CPUMetric metric(config), std::invalid_argument);
}

TEST(CPUMetricTest, CollectWithInvalidCPU) {
    json config = {{"cpu_ids", {999999}}}; // Предполагаем, что такого CPU не существует
    CPUMetric metric(config);
    ASSERT_TRUE(metric.is_valid());
    
    auto result = metric.collect();
    bool is_vector = std::holds_alternative<std::vector<double>>(result);
    EXPECT_TRUE(is_vector);
    
    if (is_vector) {
        auto usage = std::get<std::vector<double>>(result);
        EXPECT_EQ(usage.size(), 1);
        EXPECT_EQ(usage[0], 0.0); // Ожидаем 0 для несуществующего CPU
    }
}
