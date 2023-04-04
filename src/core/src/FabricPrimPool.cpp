#include "cesium/omniverse/FabricPrimPool.h"

#include "cesium/omniverse/FabricPrim.h"

#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse {

FabricPrimPool::FabricPrimPool(uint64_t id, const FabricAttributesBuilder& attributes)
    : ObjectPool<FabricPrim>()
    , _attributes(attributes)
    , _id(id) {

    const size_t initialCapacity = 200;

    for (size_t i = 0; i < initialCapacity; i++) {
        const auto path = pxr::SdfPath(fmt::format("/fabric_prim_pool_{}_item_{}", id, i));
        add(std::make_shared<FabricPrim>(path, attributes));
    }
}

void FabricPrimPool::setActive(std::shared_ptr<FabricPrim> prim, bool active) {
    prim->setActive(active);
}

const FabricAttributesBuilder& FabricPrimPool::getAttributes() const {
    return _attributes;
}

}; // namespace cesium::omniverse
