#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>
#include <vector>

using json = nlohmann::json;
using MetricValue =
    std::variant<int, double, std::vector<int>, std::vector<double>,
                 std::map<std::string, double>>;

class IMetric {
public:
    virtual ~IMetric() = default;

    virtual MetricValue collect() const = 0;

    virtual bool is_valid() const = 0;

    virtual std::string name() const = 0;
};

// Экспортируемые функции для динамической загрузки
extern "C" {
    IMetric* createMetric(const json& config);
    
    void destroyMetric(IMetric* metric);
}
