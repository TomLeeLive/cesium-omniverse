#pragma once

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <usdrt/scenegraph/usd/sdf/path.h>

namespace cesium::omniverse {
void createUsdrtPrims(
    long stageId,
    const std::string& tilesetName,
    const glm::dmat4& tileTransform,
    const CesiumGltf::Model& model);
}
