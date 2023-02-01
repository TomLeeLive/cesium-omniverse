#pragma once

#include <glm/glm.hpp>

namespace CesiumGltf {
struct Model;
}

namespace cesium::omniverse::GltfToUsd {
void createFabricPrims(
    long stageId,
    int tilesetId,
    uint64_t tileId,
    bool visible,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& tileTransform,
    const CesiumGltf::Model& model);
}
