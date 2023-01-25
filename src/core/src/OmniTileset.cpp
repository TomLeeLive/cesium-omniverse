#include "cesium/omniverse/OmniTileset.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/HttpAssetAccessor.h"
#include "cesium/omniverse/RenderResourcesPreparer.h"
#include "cesium/omniverse/TaskProcessor.h"
#include "cesium/omniverse/UsdUtil.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/IonRasterOverlay.h>
#include <Cesium3DTilesSelection/Tileset.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>

// #include <Cesium3DTilesSelection/IonRasterOverlay.h>
// #include <CesiumGeometry/AxisTransforms.h>
// #include <CesiumGltf/Material.h>
// #include <CesiumUsdSchemas/data.h>
// #include <glm/glm.hpp>

namespace cesium::omniverse {

struct InitializeTilesetResult {
    Cesium3DTilesSelection::TilesetExternals tilesetExternals;
    std::shared_ptr<RenderResourcesPreparer> renderResourcesPreparer;
    pxr::SdfPath usdPath;
    Cesium3DTilesSelection::TilesetOptions options;
};

namespace {
InitializeTilesetResult initializeTileset(const std::string& name, long stageId) {
    auto stage = getUsdStage(stageId);
    const auto usdPath = stage->GetPseudoRoot().GetPath().AppendChild(pxr::TfToken(name));
    const auto renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>();
    auto& context = Context::instance();
    auto asyncSystem = CesiumAsync::AsyncSystem(context.getTaskProcessor());
    auto externals = Cesium3DTilesSelection::TilesetExternals{
        context.getHttpAssetAccessor(),
        renderResourcesPreparer,
        asyncSystem,
        context.getCreditSystem(),
        context.getLogger()};

    Cesium3DTilesSelection::TilesetOptions options;

    options.enableFrustumCulling = false;
    options.forbidHoles = true;
    options.maximumSimultaneousTileLoads = 10;
    options.loadingDescendantLimit = 10;

    options.loadErrorCallback = [](const Cesium3DTilesSelection::TilesetLoadFailureDetails& error) {
        // Check for a 401 connecting to Cesium ion, which means the token is invalid
        // (or perhaps the asset ID is). Also check for a 404, because ion returns 404
        // when the token is valid but not authorized for the asset.
        if (error.type == Cesium3DTilesSelection::TilesetLoadType::CesiumIon &&
            (error.statusCode == 401 || error.statusCode == 404)) {
            CESIUM_LOG_ERROR("TODO: show ion access token troubleshooting window");
        }

        CESIUM_LOG_ERROR(error.message);
    };

    return InitializeTilesetResult{externals, renderResourcesPreparer, usdPath, options};
}
} // namespace

OmniTileset::OmniTileset(const std::string& name, long stageId, const std::string& url) {
    const auto& [externals, renderResourcesPreparer, usdPath, options] = initializeTileset(name, stageId);
    _renderResourcesPreparer = renderResourcesPreparer;
    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, url, options);
    _usdPath = usdPath;
}

OmniTileset::OmniTileset(const std::string& name, long stageId, int64_t ionId, const std::string& ionToken) {
    const auto& [externals, renderResourcesPreparer, usdPath, options] = initializeTileset(name, stageId);
    _renderResourcesPreparer = renderResourcesPreparer;
    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, ionId, ionToken, options);
    _usdPath = usdPath;
}

OmniTileset::~OmniTileset() {}

void OmniTileset::updateFrame(const std::vector<Cesium3DTilesSelection::ViewState>& viewStates) {
    const auto& viewUpdate = _tileset->updateView(viewStates);

    for (const auto tile : viewUpdate.tilesFadingOut) {
        if (tile->getState() == Cesium3DTilesSelection::TileLoadState::Done) {
            auto renderContent = tile->getContent().getRenderContent();
            if (renderContent) {
                auto renderResources = renderContent->getRenderResources();
                (void)renderResources;
                // TODO: setVisible(false);
            }
        }
    }

    for (const auto tile : viewUpdate.tilesToRenderThisFrame) {
        if (tile->getState() == Cesium3DTilesSelection::TileLoadState::Done) {
            const auto renderContent = tile->getContent().getRenderContent();
            if (renderContent) {
                auto renderResources = renderContent->getRenderResources();
                (void)renderResources;
                // TODO: setVisible(true);
            }
        }
    }
}

void OmniTileset::addIonRasterOverlay(const std::string& name, int64_t ionId, const std::string& ionToken) {
    Cesium3DTilesSelection::RasterOverlayOptions options;
    options.loadErrorCallback = [](const Cesium3DTilesSelection::RasterOverlayLoadFailureDetails& error) {
        // Check for a 401 connecting to Cesium ion, which means the token is invalid
        // (or perhaps the asset ID is). Also check for a 404, because ion returns 404
        // when the token is valid but not authorized for the asset.
        auto statusCode = error.pRequest && error.pRequest->response() ? error.pRequest->response()->statusCode() : 0;

        if (error.type == Cesium3DTilesSelection::RasterOverlayLoadType::CesiumIon &&
            (statusCode == 401 || statusCode == 404)) {
            CESIUM_LOG_ERROR("TODO: show ion access token troubleshooting window");
        }

        CESIUM_LOG_ERROR(error.message);
    };

    const auto rasterOverlay = new Cesium3DTilesSelection::IonRasterOverlay(name, ionId, ionToken, options);
    _tileset->getOverlays().add(rasterOverlay);
};

void OmniTileset::setTransform(const glm::dmat4& globalToLocal) {
    (void)globalToLocal;
    // TODO
}

} // namespace cesium::omniverse
