#pragma once

#include "cesium/omniverse/FabricAttributesBuilder.h"

namespace cesium::omniverse {

class FabricAttributesBuilder;
class FabricPrim;
class FabricPrimPool;

class FabricPoolManager {
  public:
    FabricPoolManager(const FabricPoolManager&) = delete;
    FabricPoolManager(FabricPoolManager&&) = delete;
    FabricPoolManager& operator=(const FabricPoolManager&) = delete;
    FabricPoolManager& operator=(FabricPoolManager) = delete;

    static FabricPoolManager& getInstance() {
        static FabricPoolManager instance;
        return instance;
    }

    std::shared_ptr<FabricPrim> acquirePrim(const FabricAttributesBuilder& attributes);
    void releasePrim(std::shared_ptr<FabricPrim> prim);

  protected:
    FabricPoolManager() = default;
    ~FabricPoolManager() = default;

  private:
    std::shared_ptr<FabricPrimPool> getPrimPool(const FabricAttributesBuilder& attributes);
    std::vector<std::shared_ptr<FabricPrimPool>> _primPools;
};

} // namespace cesium::omniverse
