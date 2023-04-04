#pragma once

#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/ObjectPool.h"

namespace cesium::omniverse {

class FabricPrim;

class FabricPrimPool : public ObjectPool<FabricPrim> {
  public:
    FabricPrimPool(uint64_t id, const FabricAttributesBuilder& attributes);
    void setActive(std::shared_ptr<FabricPrim> prim, bool active) override;

    const FabricAttributesBuilder& getAttributes() const;

  private:
    const uint64_t _id;
    const FabricAttributesBuilder _attributes;
};

} // namespace cesium::omniverse
