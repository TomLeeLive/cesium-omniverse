#include "cesium/omniverse/FabricMesh.h"

namespace cesium::omniverse {
FabricMesh::FabricMesh(
    std::shared_ptr<FabricGeometry> geometry,
    std::shared_ptr<FabricMaterial> material,
    const FabricGeometryDefinition& geometryDefinition,
    const FabricMaterialDefinition& materialDefinition)
    : _geometry(geometry)
    , _material(material)
    , _geometryDefinition(geometryDefinition)
    , _materialDefinition(materialDefinition) {}

FabricMesh::FabricMesh(std::shared_ptr<FabricGeometry> geometry, const FabricGeometryDefinition& geometryDefinition)
    : _geometry(geometry)
    , _material(nullptr)
    , _geometryDefinition(geometryDefinition) {}

std::shared_ptr<FabricGeometry> FabricMesh::getGeometry() const {
    return _geometry;
}
std::shared_ptr<FabricMaterial> FabricMesh::getMaterial() const {
    return _material;
}
const FabricGeometryDefinition& FabricMesh::getGeometryDefinition() const {
    return _geometryDefinition;
}
const FabricMaterialDefinition& FabricMesh::getMaterialDefinition() const {
    return _materialDefinition;
}

} // namespace cesium::omniverse
