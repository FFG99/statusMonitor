#include "metrics/MemoryMetric.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <set>

MemoryMetric::MemoryMetric(const json &config) {
    if (config.contains("spec") && config["spec"].is_array()) {
        std::set<std::string> unique_specs;
        bool has_duplicates = false;
        
        for (const auto &spec : config["spec"]) {
            if (!spec.is_string()) {
                continue;
            }
            std::string metric_name = spec.get<std::string>();
            if (metric_name.empty()) {
                continue;
            }
            if (!unique_specs.insert(metric_name).second) {
                has_duplicates = true;
            }
        }

        if (!has_duplicates) {
            for (const auto &name : unique_specs) {
                specs_.push_back(name);
            }
        }
    }
}

MetricValue MemoryMetric::collect() const {
    std::map<std::string, double> memory_info;
    std::ifstream meminfo_file("/proc/meminfo");
    std::string line;

    while (std::getline(meminfo_file, line)) {
        std::istringstream iss(line);
        std::string key;
        double value;
        std::string unit;

        iss >> key >> value >> unit;

        // Убираем двоеточие из ключа
        key = key.substr(0, key.length() - 1);

        // Конвертируем в МБ
        if (unit == "kB") {
            value /= 1024.0;
        }

        if (std::find(specs_.begin(), specs_.end(), key) != specs_.end()) {
            memory_info[key] = value;
        }
    }

    return memory_info;
}

bool MemoryMetric::is_valid() const {
    return !specs_.empty();
}

std::string MemoryMetric::name() const {
    return "memory";
}

extern "C" {
    IMetric* createMetric(const json& config) {
        return new MemoryMetric(config);
    }
    
    void destroyMetric(IMetric* metric) {
        delete metric;
    }
}
