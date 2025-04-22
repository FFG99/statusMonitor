#include "output/ConsoleOutput.hpp"
#include "metrics/CPUMetric.hpp"
#include "metrics/MemoryMetric.hpp"
#include <gtest/gtest.h>

TEST(ConsoleOutputTest, Write) {
    json config = json::object();
    ConsoleOutput output(config);

    std::vector<std::pair<const IMetric*, MetricValue>> metric_values;

    // Добавляем CPU метрику
    json cpu_config = {{"cpu_ids", {0, 1}}};
    auto cpu_metric = std::make_unique<CPUMetric>(cpu_config);
    metric_values.push_back({cpu_metric.get(), cpu_metric->collect()});

    // Добавляем Memory метрику
    json memory_config = {{"spec", {"MemTotal", "MemFree", "MemAvailable"}}};
    auto memory_metric = std::make_unique<MemoryMetric>(memory_config);
    metric_values.push_back({memory_metric.get(), memory_metric->collect()});

    // Проверяем, что write не выбрасывает исключений
    EXPECT_NO_THROW(output.write(metric_values));
}
