#pragma once

#include "IMetric.hpp"
#include <map>
#include <vector>

class CPUMetric : public IMetric {
public:
    explicit CPUMetric(const json &config);

    MetricValue collect() const override;
    bool is_valid() const override;
    std::string name() const override;

private:
    std::vector<int> cpu_ids_;

    struct CPUStats {
        unsigned long long user;
        unsigned long long nice;
        unsigned long long system;
        unsigned long long idle;
        unsigned long long iowait;
        unsigned long long irq;
        unsigned long long softirq;
        unsigned long long steal;
        unsigned long long guest;

        unsigned long long idle_time() const { return idle + iowait; }

        // Общее время работы
        unsigned long long total_time() const {
            return idle_time() + user + nice + system + irq + softirq + steal;
        }
    };

    // Кэш для хранения предыдущих значений
    mutable std::map<int, CPUStats> prev_stats_;

    // Флаг, указывающий, что это первое измерение
    mutable bool first_measurement_ = true;
};
