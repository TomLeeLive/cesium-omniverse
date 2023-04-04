#pragma once

#include "cesium/omniverse/FabricGeometryDefinition.h"
#include "cesium/omniverse/FabricMaterialDefinition.h"

#include <memory>

namespace cesium::omniverse {

class FabricGeometry;
class FabricMaterial;
class FabricGeometryDefinition;
class FabricMaterialDefinition;

class FabricMesh {
  public:
    FabricMesh(
        std::shared_ptr<FabricGeometry> geometry,
        std::shared_ptr<FabricMaterial> material,
        const FabricGeometryDefinition& geometryDefinition,
        const FabricMaterialDefinition& materialDefinition);

    FabricMesh(std::shared_ptr<FabricGeometry> geometry, const FabricGeometryDefinition& geometryDefinition);

    std::shared_ptr<FabricGeometry> getGeometry() const;
    std::shared_ptr<FabricMaterial> getMaterial() const;
    const FabricGeometryDefinition& getGeometryDefinition() const;
    const FabricMaterialDefinition& getMaterialDefinition() const;

  private:
    const std::shared_ptr<FabricGeometry> _geometry;
    const std::shared_ptr<FabricMaterial> _material;

    const FabricGeometryDefinition _geometryDefinition;
    const FabricMaterialDefinition _materialDefinition;
};

} // namespace cesium::omniverse
