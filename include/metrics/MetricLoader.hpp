#pragma once

#include <dlfcn.h>
#include <memory>
#include <string>
#include <vector>
#include "IMetric.hpp"

class MetricLoader {
public:
    class MetricHandle {
    public:
        MetricHandle(void* handle, IMetric* metric, void(*destroy)(IMetric*))
            : handle_(handle), metric_(metric), destroy_(destroy) {}
        
        ~MetricHandle() {
            if (metric_ && destroy_) {
                destroy_(metric_);
            }
            if (handle_) {
                dlclose(handle_);
            }
        }
        
        IMetric* get() const { return metric_; }
        
    private:
        void* handle_;
        IMetric* metric_;
        void(*destroy_)(IMetric*);
    };
    
    using MetricPtr = std::unique_ptr<MetricHandle>;
    
    static MetricPtr loadMetric(const std::string& libraryPath, const json& config) {
        void* handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
        if (!handle) {
            throw std::runtime_error("Failed to load library: " + std::string(dlerror()));
        }

        using CreateFunc = IMetric*(*)(const json&);
        using DestroyFunc = void(*)(IMetric*);

        auto createFunc = reinterpret_cast<CreateFunc>(dlsym(handle, "createMetric"));
        auto destroyFunc = reinterpret_cast<DestroyFunc>(dlsym(handle, "destroyMetric"));

        if (!createFunc || !destroyFunc) {
            dlclose(handle);
            throw std::runtime_error("Failed to load metric functions");
        }

        IMetric* metric = createFunc(config);
        if (!metric) {
            dlclose(handle);
            throw std::runtime_error("Failed to create metric");
        }

        return std::make_unique<MetricHandle>(handle, metric, destroyFunc);
    }
};
