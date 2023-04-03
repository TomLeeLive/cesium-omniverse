#include "cesium/omniverse/FabricMeshPool.h"

#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/LoggerSink.h"
#include "cesium/omniverse/Tokens.h"
#include "cesium/omniverse/UsdUtil.h"

#include <carb/flatcache/FlatCacheUSD.h>
#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse {

FabricMeshPool::FabricMeshPool() {}

void FabricMeshPool::initialize(const FabricAttributesBuilder& builder) {
    if (_items.size() > 0) {
        // Already initialized
        return;
    }

    const size_t initialCapacity = 10000;

    for (size_t i = 0; i < initialCapacity; i++) {
        const auto path = pxr::SdfPath(fmt::format("/fabric_mesh_pool_item_{}", i));
        _items.emplace_back(FabricMeshPoolItem(path, builder));
    }
}

pxr::SdfPath FabricMeshPool::get() {
    for (auto& item : _items) {
        if (!item._occupied) {
            item._occupied = true;
            return item._path;
        }
    }

    CESIUM_LOG_ERROR("Nothing left in pool");
    return {};
}

namespace {
void hidePrim(const pxr::SdfPath& path) {
    auto sip = UsdUtil::getFabricStageInProgress();
    auto worldVisibilityFabric = sip.getAttributeWr<bool>(carb::flatcache::asInt(path), FabricTokens::_worldVisibility);
    *worldVisibilityFabric = false;
}
} // namespace

void FabricMeshPool::release(const pxr::SdfPath& path) {
    // TODO: should this also set arrays sizes back to zero to free memory?
    for (auto& item : _items) {
        if (item._occupied && item._path == path) {
            hidePrim(path);
            item._occupied = false;
            return;
        }
    }

    CESIUM_LOG_ERROR("Path not found in pool");
}

FabricMeshPoolItem::FabricMeshPoolItem(const pxr::SdfPath& path, const FabricAttributesBuilder& builder)
    : _path(path)
    , _occupied(false) {
    auto sip = UsdUtil::getFabricStageInProgress();
    const auto pathFabric = carb::flatcache::Path(carb::flatcache::asInt(path));
    sip.createPrim(pathFabric);
    builder.createAttributes(pathFabric);
    auto subdivisionSchemeFabric =
        sip.getAttributeWr<carb::flatcache::Token>(pathFabric, FabricTokens::subdivisionScheme);
    *subdivisionSchemeFabric = FabricTokens::none;
}

} // namespace cesium::omniverse
