#include "cesium/omniverse/Context.h"

#include "cesium/omniverse/CoordinateSystemUtil.h"
#include "cesium/omniverse/HttpAssetAccessor.h"
#include "cesium/omniverse/LoggerSink.h"
#include "cesium/omniverse/OmniTileset.h"
#include "cesium/omniverse/TaskProcessor.h"
#include "cesium/omniverse/UsdUtil.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/CreditSystem.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/ViewState.h>
#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>
#include <pxr/usd/usdGeom/xform.h>

namespace cesium::omniverse {

namespace {} // namespace

Context& Context::instance() {
    static Context context;
    return context;
}

void Context::init(const std::filesystem::path& cesiumExtensionLocation) {
    _taskProcessor = std::make_shared<TaskProcessor>();
    _httpAssetAccessor = std::make_shared<HttpAssetAccessor>();
    _creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();

    _logger = std::make_shared<spdlog::logger>(
        std::string("cesium-omniverse"),
        spdlog::sinks_init_list{
            std::make_shared<LoggerSink>(omni::log::Level::eVerbose),
            std::make_shared<LoggerSink>(omni::log::Level::eInfo),
            std::make_shared<LoggerSink>(omni::log::Level::eWarn),
            std::make_shared<LoggerSink>(omni::log::Level::eError),
            std::make_shared<LoggerSink>(omni::log::Level::eFatal),
        });

    _tilesetIdCount = 0;

    _memCesiumPath = cesiumExtensionLocation / "bin" / "mem.cesium";
    _certificatePath = cesiumExtensionLocation / "certs";

    Cesium3DTilesSelection::registerAllTileContentTypes();
}

void Context::destroy() {
    _taskProcessor.reset();
    _httpAssetAccessor.reset();
    _creditSystem.reset();
    _logger.reset();

    _tilesetIdCount = 0;
    _tilesets.clear();

    // TODO: actually remove from USD/Fabric
}

std::shared_ptr<TaskProcessor> Context::getTaskProcessor() {
    return _taskProcessor;
}

std::shared_ptr<HttpAssetAccessor> Context::getHttpAssetAccessor() {
    return _httpAssetAccessor;
}

std::shared_ptr<Cesium3DTilesSelection::CreditSystem> Context::getCreditSystem() {
    return _creditSystem;
}

std::shared_ptr<spdlog::logger> Context::getLogger() {
    return _logger;
}

int Context::addTilesetUrl(long stageId, const std::string& url) {
    const auto tilesetId = getTilesetId();
    const auto tilesetName = fmt::format("tileset_{}", tilesetId);
    const auto tilesetUsdPath = UsdUtil::getChildOfRootPathUnique(stageId, tilesetName);
    const auto stage = UsdUtil::getUsdStage(stageId);
    pxr::UsdGeomXform::Define(stage, tilesetUsdPath);
    _tilesets.emplace_back(std::make_unique<OmniTileset>(stageId, tilesetId, tilesetUsdPath, url));
    return tilesetId;
}

int Context::addTilesetIon(long stageId, int64_t ionId, const std::string& ionToken) {
    const auto tilesetId = getTilesetId();
    const auto tilesetName = fmt::format("tileset_ion_{}", ionId);
    const auto tilesetUsdPath = UsdUtil::getChildOfRootPathUnique(stageId, tilesetName);
    const auto stage = UsdUtil::getUsdStage(stageId);
    pxr::UsdGeomXform::Define(stage, tilesetUsdPath);
    _tilesets.emplace_back(std::make_unique<OmniTileset>(stageId, tilesetId, tilesetUsdPath, ionId, ionToken));
    return tilesetId;
}

void Context::removeTileset(int tilesetId) {
    auto removedIter = std::remove_if(_tilesets.begin(), _tilesets.end(), [&tilesetId](const auto& tileset) {
        return tileset->getId() == tilesetId;
    });

    _tilesets.erase(removedIter, _tilesets.end());

    // TODO: actually remove from USD/Fabric
}

void Context::addIonRasterOverlay(int tilesetId, const std::string& name, int64_t ionId, const std::string& ionToken) {
    auto iter = std::find_if(_tilesets.begin(), _tilesets.end(), [&tilesetId](const auto& tileset) {
        return tileset->getId() == tilesetId;
    });

    if (iter != _tilesets.end()) {
        iter->get()->addIonRasterOverlay(name, ionId, ionToken);
    }
}

void Context::updateFrame(
    long stageId,
    const glm::dmat4& viewMatrix,
    const glm::dmat4& projMatrix,
    double width,
    double height) {

    _viewStates.clear();
    _viewStates.emplace_back(
        CoordinateSystemUtil::computeViewState(stageId, _georeferenceOrigin, viewMatrix, projMatrix, width, height));

    for (const auto& tileset : _tilesets) {
        const auto path = tileset->getUsdPath();
        auto& frameState = tileset->getFrameState();

        // computeEcefToUsdTransformForPrim and isPrimVisible are slightly expensive operations to do every frame
        // but they are simple and exhaustive.
        //
        // The faster approach would be to load the tileset USD prim into Fabric (via usdrt::UsdStage::GetPrimAtPath)
        // and subscribe to changed events for _worldPosition, _worldOrientation, _worldScale, and visibility.
        // Alternatively, we could register a listener with Tf::Notice but this has the downside of only notifying us
        // about changes to the current prim and not its ancestor prims. Also Tf::Notice may notify us in a thread other
        // than the main thread and we would have to be careful to synchronize updates to Fabric in the main thread.

        // Check for transform changes and update Fabric prims accordingly
        const auto ecefToUsdTransform =
            CoordinateSystemUtil::computeEcefToUsdTransformForPrim(stageId, _georeferenceOrigin, path);

        if (ecefToUsdTransform != frameState.ecefToUsdTransform) {
            frameState.ecefToUsdTransform = ecefToUsdTransform;
            UsdUtil::setTilesetTransform(stageId, tileset->getId(), ecefToUsdTransform);
        }

        // Check for visibility changes and update Fabric prims accordingly
        const auto visible = UsdUtil::isPrimVisible(stageId, path);

        if (visible != frameState.visible) {
            frameState.visible = visible;
            UsdUtil::setTilesetVisibility(stageId, tileset->getId(), visible);
        }

        if (!tileset->getSuspendUpdate()) {
            tileset->updateFrame(stageId, _viewStates);
        }
    }
}

const CesiumGeospatial::Cartographic& Context::getGeoreferenceOrigin() const {
    return _georeferenceOrigin;
}

void Context::setGeoreferenceOrigin(const CesiumGeospatial::Cartographic& origin) {
    _georeferenceOrigin = origin;
}

std::filesystem::path Context::getMemCesiumPath() const {
    return _memCesiumPath;
}

int Context::getTilesetId() {
    return _tilesetIdCount++;
}

} // namespace cesium::omniverse
