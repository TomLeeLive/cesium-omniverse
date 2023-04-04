#pragma once

#include "cesium/omniverse/FabricGeometry.h"
#include "cesium/omniverse/FabricGeometryDefinition.h"
#include "cesium/omniverse/ObjectPool.h"

namespace cesium::omniverse {

class FabricGeometryPool : public ObjectPool<FabricGeometry> {
  public:
    FabricGeometryPool(uint64_t id, const FabricGeometryDefinition& geometryDefinition);
    void setActive(std::shared_ptr<FabricGeometry> prim, bool active) override;

    const FabricGeometryDefinition& getGeometryDefinition() const;

  private:
    const uint64_t _id;
    const FabricGeometryDefinition _geometryDefinition;
};

} // namespace cesium::omniverse
