#pragma once

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/Model.h>
#include <usdrt/scenegraph/usd/sdf/path.h>

namespace cesium::omniverse {
void createUsdrtPrim(
    long stageId,
    const usdrt::SdfPath& primPath,
    const CesiumGltf::Model& model);
}
