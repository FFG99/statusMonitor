#include "metrics/CPUMetric.hpp"
#include "metrics/MemoryMetric.hpp"
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

std::vector<std::shared_ptr<IMetric>> create_metrics(const json &config) {
  std::vector<std::shared_ptr<IMetric>> metrics;

  for (const auto &metric_config : config["metrics"]) {
    try {
      std::shared_ptr<IMetric> metric;
      if (metric_config["type"] == "cpu") {
        metric = std::make_shared<CPUMetric>(metric_config);
      } else if (metric_config["type"] == "memory") {
        metric = std::make_shared<MemoryMetric>(metric_config);
      } else {
        throw std::invalid_argument("Unknown metric type: " +
                                    metric_config["type"].get<std::string>());
      }

      if (metric && metric->is_valid()) {
        metrics.push_back(metric);
        std::cout << "Added metric: " << metric->name() << std::endl;
      } else {
        throw std::runtime_error("Failed to create valid metric of type: " +
                                 metric_config["type"].get<std::string>());
      }
    } catch (const std::exception &e) {
      std::cerr << "Error creating metric: " << e.what() << std::endl;
      throw; // Перебрасываем исключение дальше
    }
  }

  return metrics;
}

std::vector<std::shared_ptr<IOutput>> create_outputs(const json &config) {
  std::vector<std::shared_ptr<IOutput>> outputs;

  for (const auto &output_config : config["outputs"]) {
    try {
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
      throw; // Перебрасываем исключение дальше
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
    std::ifstream config_file(argv[1]);
    if (!config_file.is_open()) {
      std::cerr << "Error: Cannot open config file: " << argv[1] << std::endl;
      return 1;
    }

    json config;
    try {
      config_file >> config;
    } catch (const json::exception &e) {
      std::cerr << "Error: Invalid JSON in config file: " << e.what()
                << std::endl;
      return 1;
    }

    if (!config.contains("settings") ||
        !config["settings"].contains("period")) {
      std::cerr << "Error: Config file must contain 'settings.period'"
                << std::endl;
      return 1;
    }

    if (!config.contains("metrics") || !config["metrics"].is_array() ||
        config["metrics"].empty()) {
      std::cerr << "Error: Config file must contain non-empty 'metrics' array"
                << std::endl;
      return 1;
    }

    if (!config.contains("outputs") || !config["outputs"].is_array() ||
        config["outputs"].empty()) {
      std::cerr << "Error: Config file must contain non-empty 'outputs' array"
                << std::endl;
      return 1;
    }

    auto metrics = create_metrics(config);
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

    std::cout << "\nStarting monitoring with period " << period << " seconds..."
              << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;

    while (true) {
      // Вычисляем все метрики
      std::vector<std::pair<std::shared_ptr<IMetric>, MetricValue>> metric_values;
      for (const auto &metric : metrics) {
        if (metric->is_valid()) {
          metric_values.push_back({metric, metric->collect()});
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
