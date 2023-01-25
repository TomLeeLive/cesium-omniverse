#include "cesium/omniverse/Context.h"

#include "cesium/omniverse/CoordinateSystem.h"
#include "cesium/omniverse/HttpAssetAccessor.h"
#include "cesium/omniverse/LoggerSink.h"
#include "cesium/omniverse/OmniTileset.h"
#include "cesium/omniverse/TaskProcessor.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/CreditSystem.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/ViewState.h>
#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>

namespace cesium::omniverse {

Context& Context::instance() {
    static Context context;
    return context;
}

void Context::init(const std::filesystem::path& cesiumExtensionLocation) {
    _taskProcessor = std::make_shared<TaskProcessor>();
    _httpAssetAccessor = std::make_shared<HttpAssetAccessor>();
    _creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();
    _coordinateSystem = std::make_shared<CoordinateSystem>();

    _logger = std::make_shared<spdlog::logger>(
        std::string("cesium-omniverse"),
        spdlog::sinks_init_list{
            std::make_shared<LoggerSink>(omni::log::Level::eVerbose),
            std::make_shared<LoggerSink>(omni::log::Level::eInfo),
            std::make_shared<LoggerSink>(omni::log::Level::eWarn),
            std::make_shared<LoggerSink>(omni::log::Level::eError),
            std::make_shared<LoggerSink>(omni::log::Level::eFatal),
        });

    _tilesetId = 0;

    _cesiumMemLocation = cesiumExtensionLocation / "bin";
    _certificatePath = cesiumExtensionLocation / "certs";

    Cesium3DTilesSelection::registerAllTileContentTypes();
}

void Context::destroy() {
    _taskProcessor.reset();
    _httpAssetAccessor.reset();
    _creditSystem.reset();
    _coordinateSystem.reset();
    _logger.reset();

    _tilesetId = 0;
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

std::shared_ptr<CoordinateSystem> Context::getCoordinateSystem() {
    return _coordinateSystem;
}

std::shared_ptr<spdlog::logger> Context::getLogger() {
    return _logger;
}

int Context::addTilesetUrl(long stageId, const std::string& url) {
    const auto tilesetId = getTilesetId();
    const auto tilesetName = fmt::format("tileset_{}", tilesetId);
    _tilesets.insert({tilesetId, std::make_unique<OmniTileset>(tilesetName, stageId, url)});
    return tilesetId;
}

int Context::addTilesetIon(long stageId, int64_t ionId, const std::string& ionToken) {
    const auto tilesetId = getTilesetId();
    const auto tilesetName = fmt::format("tileset_ion_{}", ionId);
    _tilesets.insert({tilesetId, std::make_unique<OmniTileset>(tilesetName, stageId, ionId, ionToken)});
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
    _viewStates.clear();

    const auto usdToEcef = getUsdToEcef(stageId, _origin);
    const auto ecefToUsd = glm::inverse(usdToEcef);
    auto inverseView = glm::inverse(viewMatrix);
    auto omniCameraUp = glm::dvec3(inverseView[1]);
    auto omniCameraFwd = glm::dvec3(-inverseView[2]);
    auto omniCameraPosition = glm::dvec3(inverseView[3]);
    auto cameraUp = glm::dvec3(usdToEcef * glm::dvec4(omniCameraUp, 0.0));
    auto cameraFwd = glm::dvec3(usdToEcef * glm::dvec4(omniCameraFwd, 0.0));
    auto cameraPosition = glm::dvec3(usdToEcef * glm::dvec4(omniCameraPosition, 1.0));

    cameraUp = glm::normalize(cameraUp);
    cameraFwd = glm::normalize(cameraFwd);

    double aspect = width / height;
    double verticalFov = 2.0 * glm::atan(1.0 / projMatrix[1][1]);
    double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);

    auto viewState = Cesium3DTilesSelection::ViewState::create(
        cameraPosition, cameraFwd, cameraUp, glm::dvec2(width, height), horizontalFov, verticalFov);

    _viewStates.emplace_back(viewState);

    for (const auto& [tilesetId, tileset] : _tilesets) {
        const auto usdPath = tileset.getUsdPath();
        const auto usdTransform = glmToGfMatrix(getUsdWorldTransform(tilesetUsdPath));
        const auto ecefToUsd = ecefToUsd * tilesetUsdTransform;

        if (usdToEcefTransform != tileset.getUsdToEcefTransform()) {
            setUsdToEcefTransform(usdToEcefTransform);
            // Update all prims
        }

        tileset->updateFrame(_viewStates);
    }
}

void Context::setGeoreferenceOrigin(const CesiumGeospatial::Cartographic& origin) {
    _origin = origin;
}

int Context::getTilesetId() {
    return _tilesetId++;
}

} // namespace cesium::omniverse
