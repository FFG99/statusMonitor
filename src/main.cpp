#include "metrics/MetricLoader.hpp"
#include "output/ConsoleOutput.hpp"
#include "output/FileOutput.hpp"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <thread>
#include <vector>

using json = nlohmann::json;

std::vector<MetricLoader::MetricPtr> create_metrics(const json &config) {
    std::vector<MetricLoader::MetricPtr> metrics;

    for (const auto &metric_config : config["metrics"]) {
        try {
            std::cout << "Loading metric: " << metric_config["type"] << std::endl;
            std::string type = metric_config["type"];
            std::string library_path = metric_config["library"].get<std::string>();
            std::cout << "Library path: " << library_path << std::endl;
            std::cout << "Config: " << metric_config["config"].dump() << std::endl;
            
            auto metric = MetricLoader::loadMetric(library_path, metric_config["config"]);
            std::cout << "Metric loaded successfully" << std::endl;
            
            if (metric && metric->get() && metric->get()->is_valid()) {
                std::cout << "Added metric: " << metric->get()->name() << std::endl;
                metrics.push_back(std::move(metric));
            } else {
                throw std::runtime_error("Failed to create valid metric of type: " + type);
            }
        } catch (const std::exception &e) {
            std::cerr << "Error creating metric: " << e.what() << std::endl;
            throw;
        }
    }

    return metrics;
}

std::vector<std::shared_ptr<IOutput>> create_outputs(const json &config) {
    std::vector<std::shared_ptr<IOutput>> outputs;

    for (const auto &output_config : config["outputs"]) {
        try {
            std::cout << "Creating output: " << output_config["type"] << std::endl;
            std::shared_ptr<IOutput> output;
            if (output_config["type"] == "console") {
                output = std::make_shared<ConsoleOutput>(output_config);
            } else if (output_config["type"] == "file") {
                output = std::make_shared<FileOutput>(output_config);
            } else {
                throw std::invalid_argument("Unknown output type: " +
                                          output_config["type"].get<std::string>());
            }

            if (output && output->is_valid()) {
                outputs.push_back(output);
                std::cout << "Added output: " << output_config["type"] << std::endl;
            } else {
                throw std::runtime_error("Failed to create valid output of type: " +
                                       output_config["type"].get<std::string>());
            }
        } catch (const std::exception &e) {
            std::cerr << "Error creating output: " << e.what() << std::endl;
            throw;
        }
    }

    return outputs;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    try {
        std::cout << "Opening config file: " << argv[1] << std::endl;
        std::ifstream config_file(argv[1]);
        if (!config_file.is_open()) {
            std::cerr << "Error: Cannot open config file: " << argv[1] << std::endl;
            return 1;
        }

        json config;
        try {
            config_file >> config;
            std::cout << "Config loaded successfully" << std::endl;
        } catch (const json::exception &e) {
            std::cerr << "Error: Invalid JSON in config file: " << e.what() << std::endl;
            return 1;
        }

        if (!config.contains("settings") || !config["settings"].contains("period")) {
            std::cerr << "Error: Config file must contain 'settings.period'" << std::endl;
            return 1;
        }

        if (!config.contains("metrics") || !config["metrics"].is_array() ||
            config["metrics"].empty()) {
            std::cerr << "Error: Config file must contain non-empty 'metrics' array" << std::endl;
            return 1;
        }

        if (!config.contains("outputs") || !config["outputs"].is_array() ||
            config["outputs"].empty()) {
            std::cerr << "Error: Config file must contain non-empty 'outputs' array" << std::endl;
            return 1;
        }

        std::cout << "Creating metrics..." << std::endl;
        auto metrics = create_metrics(config);
        std::cout << "Creating outputs..." << std::endl;
        auto outputs = create_outputs(config);

        if (metrics.empty()) {
            std::cerr << "Error: No valid metrics created" << std::endl;
            return 1;
        }

        if (outputs.empty()) {
            std::cerr << "Error: No valid outputs created" << std::endl;
            return 1;
        }

        // Получаем период из конфигурации
        int period = config["settings"]["period"].get<int>();
        if (period <= 0) {
            std::cerr << "Error: Period must be positive" << std::endl;
            return 1;
        }

        std::cout << "\nStarting monitoring with period " << period << " seconds..." << std::endl;
        std::cout << "Press Ctrl+C to stop\n" << std::endl;

        while (true) {
            // Вычисляем все метрики
            std::vector<std::pair<const IMetric*, MetricValue>> metric_values;
            for (const auto &metric : metrics) {
                if (metric && metric->get() && metric->get()->is_valid()) {
                    auto* metric_ptr = metric->get();
                    std::cout << "Collecting metric: " << metric_ptr->name() << std::endl;
                    metric_values.emplace_back(metric_ptr, metric_ptr->collect());
                }
            }

            // Передаем результаты во все выходы
            for (const auto &output : outputs) {
                if (output->is_valid()) {
                    output->write(metric_values);
                }
            }

            std::this_thread::sleep_for(std::chrono::seconds(period));
        }
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
