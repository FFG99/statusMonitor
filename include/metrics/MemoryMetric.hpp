#pragma once

#include "IMetric.hpp"
#include <vector>

class MemoryMetric : public IMetric {
public:
    explicit MemoryMetric(const json &config);

    MetricValue collect() const override;
    bool is_valid() const override;
    std::string name() const override;

private:
    std::vector<std::string> specs_;
};
