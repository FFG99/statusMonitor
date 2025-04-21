#include "metrics/CPUMetric.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>

CPUMetric::CPUMetric(const json &config) {
    if (!config.contains("cpu_ids") || !config["cpu_ids"].is_array()) {
        throw std::invalid_argument("CPU metric requires 'cpu_ids' array");
    }

    for (const auto &id : config["cpu_ids"]) {
        if (!id.is_number_integer()) {
            throw std::invalid_argument("CPU IDs must be integers");
        }
        cpu_ids_.push_back(id.get<int>());
    }

    if (cpu_ids_.empty()) {
        throw std::invalid_argument("CPU metric requires at least one CPU ID");
    }
}

MetricValue CPUMetric::collect() const {
    return collect_cpu_usage();
}

bool CPUMetric::is_valid() const {
    return !cpu_ids_.empty();
}

std::string CPUMetric::name() const {
    return "cpu";
}

std::vector<double> CPUMetric::collect_cpu_usage() const {
    std::vector<double> usage;
    std::map<int, CPUStats> current_stats;

    // Collect current values
    std::ifstream stat_file("/proc/stat");

    if (!stat_file.is_open()) {
        throw std::runtime_error("Failed to open /proc/stat");
    }

    std::string line;

    // Пропускаем строку с общей статистикой
    std::getline(stat_file, line);

    while (std::getline(stat_file, line)) {
        std::istringstream iss(line);
        std::string cpu_label;
        iss >> cpu_label;

        // Пропускаем строки, которые не относятся к CPU
        if (cpu_label.substr(0, 3) != "cpu")
            continue;

        // Извлекаем номер CPU из метки (например, "cpu0" -> 0)
        int cpu_id = std::stoi(cpu_label.substr(3));

        // Проверяем, нужно ли собирать статистику для этого CPU
        if (std::find(cpu_ids_.begin(), cpu_ids_.end(), cpu_id) != cpu_ids_.end()) {
            CPUStats stats;
            iss >> stats.user >> stats.nice >> stats.system >> stats.idle >>
                stats.iowait >> stats.irq >> stats.softirq >> stats.steal >>
                stats.guest;

            current_stats[cpu_id] = stats;
        }
    }

    if (first_measurement_) {
        prev_stats_ = current_stats;
        first_measurement_ = false;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        return collect_cpu_usage();
    }

    // Вычисляем загруженность для каждого CPU
    for (const auto &cpu_id : cpu_ids_) {
        if (current_stats.find(cpu_id) == current_stats.end() ||
            prev_stats_.find(cpu_id) == prev_stats_.end()) {
            continue;
        }

        const CPUStats &current = current_stats[cpu_id];
        const CPUStats &prev = prev_stats_[cpu_id];

        unsigned long long idle_diff = current.idle_time() - prev.idle_time();
        unsigned long long total_diff = current.total_time() - prev.total_time();

        if (total_diff == 0) {
            usage.push_back(0.0);
            continue;
        }

        // Использование CPU в процентах
        double cpu_usage = 100.0 * (total_diff - idle_diff) / total_diff;
        usage.push_back(cpu_usage);
    }

    // Сохраняем текущие значения для следующего измерения
    prev_stats_ = current_stats;

    if (usage.empty()) {
        throw std::runtime_error("Failed to collect CPU usage data");
    }

    return usage;
}