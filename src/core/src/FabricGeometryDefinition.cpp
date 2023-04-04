#include "cesium/omniverse/FabricGeometryDefinition.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/GltfUtil.h"

#include <CesiumGltf/Model.h>

namespace cesium::omniverse {

FabricGeometryDefinition::FabricGeometryDefinition(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    bool smoothNormals,
    bool hasImagery) {

    const auto hasMaterial = GltfUtil::hasMaterial(model, primitive);
    const auto hasPrimitiveSt = GltfUtil::hasPrimitiveUVs(model, primitive);
    const auto hasImagerySt = GltfUtil::hasImageryUVs(model, primitive);
    const auto hasNormals = GltfUtil::hasPrimitiveNormals(model, primitive, smoothNormals);

    _hasMaterial = hasMaterial || hasImagery;
    _hasSt = hasPrimitiveSt || hasImagerySt;
    _hasNormals = hasNormals;
}

bool FabricGeometryDefinition::operator==(const FabricGeometryDefinition& other) const {
    return std::memcmp(this, &other, sizeof(FabricGeometryDefinition)) == 0;
}

} // namespace cesium::omniverse
