#include "output/FileOutput.hpp"
#include "metrics/CPUMetric.hpp"
#include "metrics/MemoryMetric.hpp"
#include <gtest/gtest.h>
#include <fstream>

class FileOutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_file = "test_output.log";
    }

    void TearDown() override {
        std::remove(test_file.c_str());
    }

    std::string test_file;
};

TEST_F(FileOutputTest, ValidConfig) {
    json config = {{"file", test_file}};
    FileOutput output(config);
    EXPECT_TRUE(output.is_valid());
}

TEST_F(FileOutputTest, Write) {
    json config = {{"file", test_file}};
    FileOutput output(config);
    ASSERT_TRUE(output.is_valid());

    std::vector<std::pair<std::shared_ptr<IMetric>, MetricValue>> metric_values;
    
    // Добавляем CPU метрику
    json cpu_config = {{"cpu_ids", {0, 1}}};
    auto cpu_metric = std::make_shared<CPUMetric>(cpu_config);
    metric_values.push_back({cpu_metric, cpu_metric->collect()});

    // Добавляем Memory метрику
    json memory_config = {{"spec", {"MemTotal", "MemFree"}}};
    auto memory_metric = std::make_shared<MemoryMetric>(memory_config);
    metric_values.push_back({memory_metric, memory_metric->collect()});

    EXPECT_NO_THROW(output.write(metric_values));

    std::ifstream file(test_file);
    EXPECT_TRUE(file.is_open());
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    EXPECT_FALSE(content.empty());
    EXPECT_TRUE(content.find("[cpu]") != std::string::npos);
    EXPECT_TRUE(content.find("[memory]") != std::string::npos);
}