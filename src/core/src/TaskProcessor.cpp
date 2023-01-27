#include "cesium/omniverse/TaskProcessor.h"

#include <spdlog/spdlog.h>

namespace cesium::omniverse {
void TaskProcessor::startTask(std::function<void()> f) {
    // TODO: let's use carb instead
    _dispatcher.Run(f);
}
} // namespace cesium::omniverse
