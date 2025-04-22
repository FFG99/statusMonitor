#pragma once

#include "IOutput.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

class FileOutput : public IOutput {
public:
    explicit FileOutput(const json &config);
    ~FileOutput();

    void write(const std::vector<std::pair<const IMetric*, MetricValue>> &metric_values) override;
    bool is_valid() const override;

private:
    std::string file_path_;
    std::ofstream file_;
    void write_metric(const IMetric* metric, const MetricValue &value);
};
