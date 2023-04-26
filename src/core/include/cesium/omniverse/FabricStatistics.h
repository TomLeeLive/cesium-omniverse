#pragma once

#include <cstdint>

namespace cesium::omniverse {
struct FabricStatistics {
    uint64_t numberOfMaterialsLoaded{0};
    uint64_t numberOfGeometriesLoaded{0};
    uint64_t numberOfGeometriesVisible{0};
};
} // namespace cesium::omniverse
