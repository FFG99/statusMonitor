#pragma once

#include "IOutput.hpp"
#include <nlohmann/json.hpp>

class ConsoleOutput : public IOutput {
public:
    explicit ConsoleOutput(const json &config);

    void write(const std::vector<std::pair<std::shared_ptr<IMetric>, MetricValue>> &metric_values) override;
    bool is_valid() const override;

private:
    void print_metric(const std::shared_ptr<IMetric> &metric, const MetricValue &value) const;
};
