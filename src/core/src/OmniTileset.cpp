#include "cesium/omniverse/OmniTileset.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/HttpAssetAccessor.h"
#include "cesium/omniverse/RenderResourcesPreparer.h"
#include "cesium/omniverse/TaskProcessor.h"
#include "cesium/omniverse/Util.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <Cesium3DTilesSelection/TilesetExternals.h>
#include <pxr/usd/usd/stage.h>

// #include <Cesium3DTilesSelection/IonRasterOverlay.h>
// #include <CesiumGeometry/AxisTransforms.h>
// #include <CesiumGltf/Material.h>
// #include <CesiumUsdSchemas/data.h>
// #include <glm/glm.hpp>

namespace cesium::omniverse {

OmniTileset::OmniTileset(int tilesetId, long stageId, const std::string& url) {
    auto& context = Context::instance();
    auto& taskProcessor = context.getTaskProcessor();
    auto& httpAssetAccessor = context.getHttpAssetAccessor();
    auto& creditSystem = context.getCreditSystem();
    auto& stage = getStage(stageId);

    const auto tilesetName = pxr::TfToken(fmt::format("tileset_{}", tilesetId));
    const auto tilesetPath = stage->GetPseudoRoot().GetPath().AppendChild(tilesetName);
    _renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(stage, tilesetPath);
    auto asyncSystem = CesiumAsync::AsyncSystem(taskProcessor);
    auto externals = Cesium3DTilesSelection::TilesetExternals{
        httpAssetAccessor, _renderResourcesPreparer, asyncSystem, creditSystem, spdlog::default_logger};

    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, url);
    _tileset->
}

OmniTileset::OmniTileset(long stageId, int64_t ionId, const std::string& ionToken) {
    auto& context = Context::instance();
    auto& taskProcessor = context.getTaskProcessor();
    auto& httpAssetAccessor = context.getHttpAssetAccessor();
    auto& creditSystem = context.getCreditSystem();
    auto& stage = getStage(stageId);

    const auto tilesetName = pxr::TfToken(fmt::format("tileset_ion_{}", ionId));
    const auto tilesetPath = stage->GetPseudoRoot().GetPath().AppendChild(tilesetName);
    _renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(stage, tilesetPath);
    auto asyncSystem = CesiumAsync::AsyncSystem(taskProcessor);
    auto externals = Cesium3DTilesSelection::TilesetExternals{
        httpAssetAccessor, _renderResourcesPreparer, asyncSystem, creditSystem};

    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, ionId, ionToken);
}

void OmniTileset::updateFrame(const std::vector<Cesium3DTilesSelection::ViewState>& viewStates) {
    const auto& viewUpdate = _tileset->updateView(viewStates);

    // Retrieve tiles are visible in the current frame
    for (const auto tile : viewUpdate.tilesFadingOut) {
        if (tile->getState() == Cesium3DTilesSelection::TileLoadState::Done) {
            auto renderContent = tile->getContent().getRenderContent();
            if (renderContent) {
                auto renderResources = renderContent->getRenderResources();
                _renderResourcesPreparer->setVisible(renderResources, false);
            }
        }
    }

    for (const auto tile : viewUpdate.tilesToRenderThisFrame) {
        if (tile->getState() == Cesium3DTilesSelection::TileLoadState::Done) {
            const auto renderContent = tile->getContent().getRenderContent();
            if (renderContent) {
                auto renderResources = renderContent->getRenderResources();
                _renderResourcesPreparer->setVisible(renderResources, true);
            }
        }
    }
}

void OmniTileset::addIonRasterOverlay(const std::string& name, int64_t ionId, const std::string& ionToken) {
    Cesium3DTilesSelection::RasterOverlayOptions options;
    options.loadErrorCallback = [](const Cesium3DTilesSelection::RasterOverlayLoadFailureDetails& error) {
        spdlog::default_logger()->error("Raster overlay failed");
        spdlog::default_logger()->error("Url: {}", error.pRequest->url);
    };

    rasterOverlay = new Cesium3DTilesSelection::IonRasterOverlay(name, ionId, ionToken, options);
    tileset->getOverlays().add(rasterOverlay);
}

void OmniTileset::init(const std::filesystem::path& cesiumExtensionLocation) {
    GltfToUSD::CesiumMemLocation = cesiumExtensionLocation / "bin";
#ifdef CESIUM_OMNI_UNIX
    HttpAssetAccessor::CertificatePath = cesiumExtensionLocation / "certs";
#endif

    auto logger = spdlog::default_logger();
    logger->sinks().clear();
    logger->sinks().push_back(std::make_shared<LoggerSink>());

    taskProcessor = std::make_shared<TaskProcessor>();
    httpAssetAccessor = std::make_shared<HttpAssetAccessor>();
    creditSystem = std::make_shared<Cesium3DTilesSelection::CreditSystem>();
    Cesium3DTilesSelection::registerAllTileContentTypes();
}

void OmniTileset::shutdown() {
    taskProcessor.reset();
    httpAssetAccessor.reset();
    creditSystem.reset();
}

void OmniTileset::initOriginShiftHandler() {
    originShiftHandler.set_callback(
        [this]([[maybe_unused]] const glm::dmat4& localToGlobal, const glm::dmat4& globalToLocal) {
            this->renderResourcesPreparer->setTransform(globalToLocal);
        });

    originShiftHandler.connect(Georeference::instance().originChangeEvent);
    this->renderResourcesPreparer->setTransform(Georeference::instance().globalToLocal);
}
} // namespace cesium::omniverse
