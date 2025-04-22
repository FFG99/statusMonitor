#include "metrics/MemoryMetric.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

MemoryMetric::MemoryMetric(const json &config) {
    if (config.contains("spec") && config["spec"].is_array()) {
        for (const auto &spec : config["spec"]) {
            if (spec.is_string()) {
                specs_.push_back(spec.get<std::string>());
            }
        }
    }
}

MetricValue MemoryMetric::collect() const { return get_memory_info(); }

bool MemoryMetric::is_valid() const { return !specs_.empty(); }

std::string MemoryMetric::name() const { return "memory"; }

std::map<std::string, double> MemoryMetric::get_memory_info() const {
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

extern "C" {
    IMetric* createMetric(const json& config) {
        return new MemoryMetric(config);
    }
    
    void destroyMetric(IMetric* metric) {
        delete metric;
    }
}
