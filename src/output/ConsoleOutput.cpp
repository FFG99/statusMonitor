#include "output/ConsoleOutput.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

ConsoleOutput::ConsoleOutput(const json &config) {
    // В будущем здесь может быть дополнительная конфигурация
    // Например, формат вывода
}

void ConsoleOutput::write(
    const std::vector<std::pair<const IMetric*, MetricValue>> &metric_values) {
    std::system("clear");
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::ctime(&time);
    std::string time_str = ss.str();

    // Перед выводом заголовка убераем перенос строки
    if (!time_str.empty() && time_str.back() == '\n') {
        time_str.pop_back();
    }
    std::cout << "=== System Metrics at " << time_str << " ===\n\n";

    for (const auto &[metric, value] : metric_values) {
        if (metric->is_valid()) {
            print_metric(metric, value);
            std::cout << "\n";
        }
    }

    std::cout << std::flush;
}

bool ConsoleOutput::is_valid() const {
    return true; // Сейчас консольный вывод всегда валиден
}

void ConsoleOutput::print_metric(const IMetric* metric, const MetricValue &value) const {
    std::cout << "[" << metric->name() << "]\n";
    std::cout << "-------------------------------------\n";

    if (std::holds_alternative<std::vector<double>>(value)) {
        auto usage = std::get<std::vector<double>>(value);
        for (size_t i = 0; i < usage.size(); ++i) {
            std::cout << "CPU " << i << ": " << std::fixed << std::setprecision(2)
                    << usage[i] << "%\n";
        }
    } else if (std::holds_alternative<std::map<std::string, double>>(value)) {
        auto memory = std::get<std::map<std::string, double>>(value);
        for (const auto &[key, val] : memory) {
            std::cout << key << ": " << std::fixed << std::setprecision(2) << val
                    << " MB\n";
        }
    }
}
