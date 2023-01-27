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
#include <Cesium3DTilesSelection/ViewState.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/xform.h>

namespace cesium::omniverse {

struct InitializeTilesetResult {
    Cesium3DTilesSelection::TilesetExternals tilesetExternals;
    std::shared_ptr<RenderResourcesPreparer> renderResourcesPreparer;
    Cesium3DTilesSelection::TilesetOptions options;
};

namespace {
InitializeTilesetResult initializeTileset(long stageId, const pxr::SdfPath& usdPath, const OmniTileset& tileset) {
    const auto stage = UsdUtil::getUsdStage(stageId);
    pxr::UsdGeomXform::Define(stage, usdPath);
    const auto renderResourcesPreparer = std::make_shared<RenderResourcesPreparer>(stageId, tileset);
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

    return InitializeTilesetResult{externals, renderResourcesPreparer, options};
}
} // namespace

OmniTileset::OmniTileset(long stageId, int tilesetId, const pxr::SdfPath& usdPath, const std::string& url) {
    const auto& [externals, renderResourcesPreparer, options] = initializeTileset(stageId, usdPath, *this);
    _renderResourcesPreparer = renderResourcesPreparer;
    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, url, options);
    _usdPath = usdPath;
    _id = tilesetId;
}

OmniTileset::OmniTileset(
    long stageId,
    int tilesetId,
    const pxr::SdfPath& usdPath,
    int64_t ionId,
    const std::string& ionToken) {
    const auto& [externals, renderResourcesPreparer, options] = initializeTileset(stageId, usdPath, *this);
    _renderResourcesPreparer = renderResourcesPreparer;
    _tileset = std::make_unique<Cesium3DTilesSelection::Tileset>(externals, ionId, ionToken, options);
    _usdPath = usdPath;
    _id = tilesetId;
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
    (void)name;
    (void)ionId;
    (void)ionToken;
    // Cesium3DTilesSelection::RasterOverlayOptions options;
    // options.loadErrorCallback = [](const Cesium3DTilesSelection::RasterOverlayLoadFailureDetails& error) {
    //     // Check for a 401 connecting to Cesium ion, which means the token is invalid
    //     // (or perhaps the asset ID is). Also check for a 404, because ion returns 404
    //     // when the token is valid but not authorized for the asset.
    //     auto statusCode = error.pRequest && error.pRequest->response() ? error.pRequest->response()->statusCode() : 0;

    //     if (error.type == Cesium3DTilesSelection::RasterOverlayLoadType::CesiumIon &&
    //         (statusCode == 401 || statusCode == 404)) {
    //         CESIUM_LOG_ERROR("TODO: show ion access token troubleshooting window");
    //     }

    //     CESIUM_LOG_ERROR(error.message);
    // };

    // const auto rasterOverlay = new Cesium3DTilesSelection::IonRasterOverlay(name, ionId, ionToken, options);
    // _tileset->getOverlays().add(rasterOverlay);
};

const pxr::SdfPath& OmniTileset::getUsdPath() const {
    return _usdPath;
}

const int OmniTileset::getId() const {
    return _id;
}

const glm::dmat4& OmniTileset::getEcefToUsdTransform() const {
    return _ecefToUsdTransform;
}

void OmniTileset::setEcefToUsdTransform(const glm::dmat4& ecefToUsdTransform) {
    _ecefToUsdTransform = ecefToUsdTransform;
}

} // namespace cesium::omniverse
