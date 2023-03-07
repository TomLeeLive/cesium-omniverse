#include "cesium/omniverse/UsdNotificationHandler.h"

#include "cesium/omniverse/AssetRegistry.h"
#include "cesium/omniverse/LoggerSink.h"
#include "cesium/omniverse/OmniImagery.h"
#include "cesium/omniverse/OmniTileset.h"
#include "cesium/omniverse/UsdUtil.h"

#include <pxr/usd/usd/primRange.h>

namespace cesium::omniverse {

namespace {
ChangedPrimType getType(const pxr::SdfPath& path) {
    if (UsdUtil::primExists(path)) {
        if (UsdUtil::isCesiumData(path)) {
            return ChangedPrimType::CESIUM_DATA;
        } else if (UsdUtil::isCesiumTileset(path)) {
            return ChangedPrimType::CESIUM_TILESET;
        } else if (UsdUtil::isCesiumImagery(path)) {
            return ChangedPrimType::CESIUM_IMAGERY;
        }
    } else {
        // If the prim doesn't exist (because it was removed from the stage already) we can get the type from the asset registry
        const auto assetType = AssetRegistry::getInstance().getAssetType(path);

        switch (assetType) {
            case AssetType::TILESET:
                return ChangedPrimType::CESIUM_TILESET;
            case AssetType::IMAGERY:
                return ChangedPrimType::CESIUM_IMAGERY;
            case AssetType::OTHER:
                break;
            default:
                break;
        }
    }

    return ChangedPrimType::OTHER;
}

bool inSubtree(const pxr::SdfPath& path, const pxr::SdfPath& descendantPath) {
    if (path == descendantPath) {
        return true;
    }

    for (const auto& ancestorPath : descendantPath.GetAncestorsRange()) {
        if (ancestorPath == path) {
            return true;
        }
    }

    return false;
}

} // namespace

UsdNotificationHandler::UsdNotificationHandler() {
    _noticeListenerKey = pxr::TfNotice::Register(pxr::TfCreateWeakPtr(this), &UsdNotificationHandler::onObjectsChanged);
}

UsdNotificationHandler::~UsdNotificationHandler() {
    pxr::TfNotice::Revoke(_noticeListenerKey);
}

std::vector<ChangedPrim> UsdNotificationHandler::popChangedPrims() {
    static const std::vector<ChangedPrim> empty;

    if (_changedPrims.size() == 0) {
        return empty;
    }

    std::vector<ChangedPrim> changedPrims;

    changedPrims.insert(
        changedPrims.end(),
        std::make_move_iterator(_changedPrims.begin()),
        std::make_move_iterator(_changedPrims.end()));

    _changedPrims.erase(_changedPrims.begin(), _changedPrims.end());

    return changedPrims;
}

void UsdNotificationHandler::onObjectsChanged(const pxr::UsdNotice::ObjectsChanged& objectsChanged) {
    if (!UsdUtil::hasStage()) {
        return;
    }

    const auto& resyncedPaths = objectsChanged.GetResyncedPaths();
    for (const auto& path : resyncedPaths) {
        if (path.IsPrimPath()) {
            if (UsdUtil::primExists(path)) {
                onPrimAdded(path);
            } else {
                onPrimRemoved(path);
            }
        }
    }

    const auto& changedPaths = objectsChanged.GetChangedInfoOnlyPaths();
    for (const auto& path : changedPaths) {
        if (path.IsPropertyPath()) {
            onPropertyChanged(path);
        }
    }
}

void UsdNotificationHandler::onPrimAdded(const pxr::SdfPath& primPath) {
    const auto type = getType(primPath);
    if (type != ChangedPrimType::OTHER) {
        _changedPrims.emplace_back(ChangedPrim{primPath, pxr::TfToken(), type, ChangeType::PRIM_ADDED});
        CESIUM_LOG_INFO("Added prim: {}", primPath.GetText());

        // USD only notifies us about the top-most prim. Traverse over descendant prims and add those as well.
        // This comes up when a tileset with imagery is moved or renamed.
        const auto stage = UsdUtil::getUsdStage();
        const auto prim = stage->GetPrimAtPath(primPath);
        for (const auto& descendant : prim.GetAllDescendants()) {
            onPrimAdded(descendant.GetPath());
        }
    }
}

void UsdNotificationHandler::onPrimRemoved(const pxr::SdfPath& primPath) {
    // USD only notifies us about the top-most prim. This prim may have tileset / imagery descendants that need to
    // be removed as well. Unlike onPrimAdded we can't traverse the stage because these prims no longer exist. Instead
    // loop through tilesets and imagery in the asset registry.
    const auto& tilesets = AssetRegistry::getInstance().getAllTilesets();
    for (const auto& tileset : tilesets) {
        const auto tilesetPath = tileset->getPath();
        const auto type = getType(tilesetPath);
        if (type != ChangedPrimType::OTHER) {
            if (inSubtree(primPath, tilesetPath)) {
                _changedPrims.emplace_back(ChangedPrim{tilesetPath, pxr::TfToken(), type, ChangeType::PRIM_REMOVED});
                CESIUM_LOG_INFO("Removed prim: {}", tilesetPath.GetText());
            }
        }
    }

    const auto& imageries = AssetRegistry::getInstance().getAllImageries();
    for (const auto& imagery : imageries) {
        const auto imageryPath = imagery->getPath();
        const auto type = getType(imageryPath);
        if (type != ChangedPrimType::OTHER) {
            if (inSubtree(primPath, imageryPath)) {
                _changedPrims.emplace_back(ChangedPrim{imageryPath, pxr::TfToken(), type, ChangeType::PRIM_REMOVED});
                CESIUM_LOG_INFO("Removed prim: {}", imageryPath.GetText());
            }
        }
    }
}

void UsdNotificationHandler::onPropertyChanged(const pxr::SdfPath& propertyPath) {
    const auto& propertyName = propertyPath.GetNameToken();
    const auto& primPath = propertyPath.GetPrimPath();
    const auto& type = getType(primPath);
    if (type != ChangedPrimType::OTHER) {
        _changedPrims.emplace_back(ChangedPrim{primPath, propertyName, type, ChangeType::PROPERTY_CHANGED});
        CESIUM_LOG_INFO("Changed property: {}", propertyPath.GetText());
    }
}

} // namespace cesium::omniverse
