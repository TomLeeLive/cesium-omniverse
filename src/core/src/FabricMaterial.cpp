void FabricGeometry::setActive(bool active) {
    if (!active) {
        // TODO: should this also set arrays sizes back to zero to free memory?
        auto sip = UsdUtil::getFabricStageInProgress();
        auto worldVisibility = sip.getAttributeWr<bool>(carb::flatcache::asInt(_path), FabricTokens::_worldVisibility);
        *worldVisibility = false;

        // Make sure to unset the tileset and tile properties
    }
}
