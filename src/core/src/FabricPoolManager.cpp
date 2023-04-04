#include "cesium/omniverse/FabricPoolManager.h"

#include "cesium/omniverse/FabricPrim.h"
#include "cesium/omniverse/FabricPrimPool.h"

namespace cesium::omniverse {

std::shared_ptr<FabricPrim> FabricPoolManager::acquirePrim(const FabricAttributesBuilder& attributes) {
    const auto primPool = getPrimPool(attributes);
    return primPool->acquire();
}

void FabricPoolManager::releasePrim(std::shared_ptr<FabricPrim> prim) {
    const auto primPool = getPrimPool(prim->getAttributes());
    return primPool->release(prim);
}

std::shared_ptr<FabricPrimPool> FabricPoolManager::getPrimPool(const FabricAttributesBuilder& attributes) {
    // Find a pool with the same attributes
    for (const auto primPool : _primPools) {
        if (FabricAttributesBuilder::isEqual(attributes, primPool->getAttributes())) {
            return primPool;
        }
    }

    // Otherwise, create a new pool
    const auto id = _primPools.size();
    return _primPools.emplace_back(std::make_shared<FabricPrimPool>(id, attributes));
}

}; // namespace cesium::omniverse
