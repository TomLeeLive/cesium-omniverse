#pragma once

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <usdrt/scenegraph/usd/sdf/path.h>

namespace cesium::omniverse::GltfToUsd {
void createUsdrtPrims(
    long stageId,
    int tilesetId,
    uint64_t tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& tileTransform,
    const std::string& tilesetName,
    const CesiumGltf::Model& model);
}
