#pragma once

#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/ObjectPool.h"

namespace cesium::omniverse {

class FabricMaterial;

class FabricMaterialPool : public ObjectPool<FabricMaterial> {
  public:
    FabricMaterialPool(uint64_t id, const FabricMaterialDefinition& materialDefinition);

    const FabricMaterialDefinition& getMaterialDefinition() const;

  private:
    const uint64_t _id;
    const FabricMaterialDefinition _materialDefinition;
};

} // namespace cesium::omniverse
