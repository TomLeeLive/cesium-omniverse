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
#include <Cesium3DTilesSelection/ViewState.h>
#include <Cesium3DTilesSelection/registerAllTileContentTypes.h>

namespace cesium::omniverse {

Context& Context::instance() {
    static Context context;
    return context;
}

void Context::init(const std::filesystem::path& cesiumExtensionLocation) {
    const auto verboseLoggerSink = std::make_shared<LoggerSink>(omni::log::Level::eVerbose);
    const auto infoLoggerSink = std::make_shared<LoggerSink>(omni::log::Level::eInfo);
    const auto warnLoggerSink = std::make_shared<LoggerSink>(omni::log::Level::eWarn);
    const auto errorLoggerSink = std::make_shared<LoggerSink>(omni::log::Level::eError);
    const auto fatalLoggerSink = std::make_shared<LoggerSink>(omni::log::Level::eFatal);

    verboseLoggerSink->set_level(spdlog::level::trace);
    infoLoggerSink->set_level(spdlog::level::info);
    warnLoggerSink->set_level(spdlog::level::warn);
    errorLoggerSink->set_level(spdlog::level::err);
    fatalLoggerSink->set_level(spdlog::level::critical);

    auto logger = spdlog::default_logger();
    logger->sinks().clear();
    logger->sinks().push_back(verboseLoggerSink);
    logger->sinks().push_back(infoLoggerSink);
    logger->sinks().push_back(warnLoggerSink);
    logger->sinks().push_back(errorLoggerSink);
    logger->sinks().push_back(fatalLoggerSink);

    _taskProcessor = std::make_shared<TaskProcessor>();
    _httpAssetAccessor = std::make_shared<HttpAssetAccessor>();
    _creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();
    _coordinateSystem = std::make_shared<CoordinateSystem>();

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

    _tilesets.clear();
    _viewStates.clear();
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

void Context::updateFrame(const glm::dmat4& viewMatrix, const glm::dmat4& projMatrix, double width, double height) {
    _viewStates.clear();

    const auto& localToGlobal = _coordinateSystem->getLocalToGlobal();
    auto inverseView = glm::inverse(viewMatrix);
    auto omniCameraUp = glm::dvec3(inverseView[1]);
    auto omniCameraFwd = glm::dvec3(-inverseView[2]);
    auto omniCameraPosition = glm::dvec3(inverseView[3]);
    auto cameraUp = glm::dvec3(localToGlobal * glm::dvec4(omniCameraUp, 0.0));
    auto cameraFwd = glm::dvec3(localToGlobal * glm::dvec4(omniCameraFwd, 0.0));
    auto cameraPosition = glm::dvec3(localToGlobal * glm::dvec4(omniCameraPosition, 1.0));

    cameraUp = glm::normalize(cameraUp);
    cameraFwd = glm::normalize(cameraFwd);

    double aspect = width / height;
    double verticalFov = 2.0 * glm::atan(1.0 / projMatrix[1][1]);
    double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);

    auto viewState = Cesium3DTilesSelection::ViewState::create(
        cameraPosition, cameraFwd, cameraUp, glm::dvec2(width, height), horizontalFov, verticalFov);

    _viewStates.emplace_back(viewState);

    for (const auto& [tilesetId, tileset] : _tilesets) {
        tileset->updateFrame();
    }
}

int Context::getTilesetId() {
    return _tilesetId++;
}

} // namespace cesium::omniverse
