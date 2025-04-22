#include "output/FileOutput.hpp"
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

FileOutput::FileOutput(const json &config) {
    if (config.contains("file") && config["file"].is_string()) {
        file_path_ = config["file"].get<std::string>();
    } else if (config.contains("path") && config["path"].is_string()) {
        file_path_ = config["path"].get<std::string>();
    }

    if (!file_path_.empty()) {
        file_.open(file_path_, std::ios::app);
    }
}

FileOutput::~FileOutput() {
    if (file_.is_open()) {
        file_.close();
    }
}

void FileOutput::write(const std::vector<std::pair<const IMetric*, MetricValue>> &metric_values) {
    if (!file_.is_open()) {
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::ctime(&time);
    std::string time_str = ss.str();
    if (!time_str.empty() && time_str.back() == '\n') {
        time_str.pop_back();
    }
    file_ << "\n=== Metrics at " << time_str << " ===\n";

    for (const auto &[metric, value] : metric_values) {
        if (metric->is_valid()) {
            write_metric(metric, value);
        }
    }

    file_ << std::endl;
    file_.flush();
}

bool FileOutput::is_valid() const { return file_.is_open(); }

void FileOutput::write_metric(const IMetric* metric, const MetricValue &value) {
    file_ << "\n[" << metric->name() << "]\n";

    if (std::holds_alternative<std::vector<double>>(value)) {
        auto usage = std::get<std::vector<double>>(value);
        for (size_t i = 0; i < usage.size(); ++i) {
            file_ << "CPU " << i << ": " << std::fixed << std::setprecision(2)
                << usage[i] << "%\n";
        }
    } else if (std::holds_alternative<std::map<std::string, double>>(value)) {
        auto memory = std::get<std::map<std::string, double>>(value);
        for (const auto &[key, val] : memory) {
            file_ << key << ": " << std::fixed << std::setprecision(2) << val
                << " MB\n";
        }
    }
}
