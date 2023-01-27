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

    _cesiumMemLocation = cesiumExtensionLocation / "bin";
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
    const auto tilesetUsdPath = getChildOfRootPathUnique(stageId, tilesetName);
    _tilesets.insert({tilesetId, std::make_unique<OmniTileset>(stageId, tilesetUsdPath, url)});
    return tilesetId;
}

int Context::addTilesetIon(long stageId, int64_t ionId, const std::string& ionToken) {
    const auto tilesetId = getTilesetId();
    const auto tilesetName = fmt::format("tileset_ion_{}", ionId);
    const auto tilesetUsdPath = getChildOfRootPathUnique(stageId, tilesetName);
    _tilesets.insert({tilesetId, std::make_unique<OmniTileset>(stageId, tilesetUsdPath, ionId, ionToken)});
    return tilesetId;
}

void Context::removeTileset(int tileset) {
    // TODO: actually remove from USD/Fabric
    _tilesets.erase(tileset);
}

void Context::addIonRasterOverlay(int tileset, const std::string& name, int64_t ionId, const std::string& ionToken) {
    const auto iter = _tilesets.find(tileset);
    if (iter != _tilesets.end()) {
        iter->second->addIonRasterOverlay(name, ionId, ionToken);
    }
}

void Context::updateFrame(
    long stageId,
    const glm::dmat4& viewMatrix,
    const glm::dmat4& projMatrix,
    double width,
    double height) {
    const auto viewState = computeViewState(stageId, _georeferenceOrigin, viewMatrix, projMatrix, width, height);
    _viewStates.clear();
    _viewStates.emplace_back(viewState);

    for (const auto& [tilesetId, tileset] : _tilesets) {
        const auto transform = computeEcefToUsdTransformForPrim(stageId, _georeferenceOrigin, tileset->getUsdPath());
        if (transform != tileset->getEcefToUsdTransform()) {
            tileset->setEcefToUsdTransform(transform);
            updatePrimTransforms(stageId, tilesetId, transform);
        }

        tileset->updateFrame(_viewStates);
    }
}

void Context::setGeoreferenceOrigin(const CesiumGeospatial::Cartographic& origin) {
    _georeferenceOrigin = origin;
}

int Context::getTilesetId() {
    return _tilesetIdCount++;
}

} // namespace cesium::omniverse
