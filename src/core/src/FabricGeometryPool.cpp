#include "cesium/omniverse/FabricGeometryPool.h"

#include "cesium/omniverse/FabricGeometry.h"

#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse {

FabricGeometryPool::FabricGeometryPool(uint64_t id, const FabricAttributesBuilder& attributes)
    : ObjectPool<FabricGeometry>()
    , _attributes(attributes)
    , _id(id) {

    const size_t initialCapacity = 200;

    for (size_t i = 0; i < initialCapacity; i++) {
        // TODO: will this work if the stage is reset?
        const auto path = pxr::SdfPath(fmt::format("/fabric_prim_pool_{}_item_{}", id, i));
        add(std::make_shared<FabricGeometry>(path, attributes));
    }
}

void FabricGeometryPool::setActive(std::shared_ptr<FabricGeometry> prim, bool active) {
    prim->setActive(active);
}

const FabricAttributesBuilder& FabricGeometryPool::getAttributes() const {
    return _attributes;
}

}; // namespace cesium::omniverse
