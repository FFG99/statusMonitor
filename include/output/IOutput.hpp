#pragma once

#include "metrics/IMetric.hpp"
#include <memory>
#include <string>
#include <vector>

class IOutput {
public:
    virtual ~IOutput() = default;

    // Вывод метрик с уже вычисленными значениями
    virtual void write(const std::vector<std::pair<const IMetric*, MetricValue>> &metric_values) = 0;

    // Проверка валидности конфигурации
    virtual bool is_valid() const = 0;
};
