#pragma once

#include "IOutput.hpp"
#include <nlohmann/json.hpp>

class ConsoleOutput : public IOutput {
public:
    explicit ConsoleOutput(const json &config);

    void write(const std::vector<std::pair<const IMetric*, MetricValue>> &metric_values) override;
    bool is_valid() const override;

private:
    void print_metric(const IMetric* metric, const MetricValue &value) const;
};
