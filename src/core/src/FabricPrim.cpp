#include "cesium/omniverse/FabricPrim.h"

#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/Tokens.h"
#include "cesium/omniverse/UsdUtil.h"

#include <carb/flatcache/FlatCacheUSD.h>

namespace cesium::omniverse {

namespace {
void deletePrimsFabric(const std::vector<pxr::SdfPath>& primsToDelete) {
    // Prims removed from Fabric need special handling for their removal to be reflected in the Hydra render index
    // This workaround may not be needed in future Kit versions, but is needed as of Kit 104.2
    auto sip = UsdUtil::getFabricStageInProgress();

    const carb::flatcache::Path changeTrackingPath("/TempChangeTracking");

    if (sip.getAttribute<uint64_t>(changeTrackingPath, FabricTokens::_deletedPrims) == nullptr) {
        return;
    }

    const auto deletedPrimsSize = sip.getArrayAttributeSize(changeTrackingPath, FabricTokens::_deletedPrims);
    sip.setArrayAttributeSize(changeTrackingPath, FabricTokens::_deletedPrims, deletedPrimsSize + primsToDelete.size());
    auto deletedPrimsFabric = sip.getArrayAttributeWr<uint64_t>(changeTrackingPath, FabricTokens::_deletedPrims);

    for (size_t i = 0; i < primsToDelete.size(); i++) {
        deletedPrimsFabric[deletedPrimsSize + i] = carb::flatcache::asInt(primsToDelete[i]).path;
    }
}

} // namespace

FabricPrim::FabricPrim(pxr::SdfPath geomPath, const FabricAttributesBuilder& attributes)
    : _attributes(attributes) {
    // TODO: is it ok to use the same name after a stage refresh occurs?
    auto sip = UsdUtil::getFabricStageInProgress();
    const auto geomPathFabric = carb::flatcache::Path(carb::flatcache::asInt(geomPath));
    sip.createPrim(geomPathFabric);
    attributes.createAttributes(geomPathFabric);
    auto subdivisionSchemeFabric =
        sip.getAttributeWr<carb::flatcache::Token>(geomPathFabric, FabricTokens::subdivisionScheme);
    *subdivisionSchemeFabric = FabricTokens::none;
}

FabricPrim::~FabricPrim() {
    // Only delete prims if there's still a stage to delete them from
    if (!UsdUtil::hasStage()) {
        return;
    }

    auto sip = UsdUtil::getFabricStageInProgress();

    sip.destroyPrim(carb::flatcache::asInt(_geomPath));

    for (const auto& materialPath : _materialsPaths) {
        sip.destroyPrim(carb::flatcache::asInt(materialPath));
    }

    deletePrimsFabric({_geomPath});
    deletePrimsFabric(_materialsPaths);
}

void FabricPrim::setActive(bool active) {
    if (!active) {
        // TODO: should this also set arrays sizes back to zero to free memory?
        auto sip = UsdUtil::getFabricStageInProgress();
        auto worldVisibilityFabric =
            sip.getAttributeWr<bool>(carb::flatcache::asInt(_geomPath), FabricTokens::_worldVisibility);
        *worldVisibilityFabric = false;
    }
}

pxr::SdfPath FabricPrim::getPath() const {
    return _geomPath;
}

const FabricAttributesBuilder& FabricPrim::getAttributes() const {
    return _attributes;
}

}; // namespace cesium::omniverse
