#include "cesium/omniverse/RenderResourcesPreparer.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/CoordinateSystemUtil.h"
#include "cesium/omniverse/GltfToUSD.h"
#include "cesium/omniverse/OmniTileset.h"
#include "cesium/omniverse/RenderResourcesPreparer.h"
#include "cesium/omniverse/UsdUtil.h"

#include <CesiumAsync/AsyncSystem.h>

namespace cesium::omniverse {

namespace {
void createFabricPrims(
    int stageId,
    const OmniTileset& tileset,
    uint64_t tileId,
    const glm::dmat4& transform,
    const CesiumGltf::Model& model) {

    const auto path = tileset.getUsdPath();
    const auto ecefToUsdTransform = CoordinateSystemUtil::computeEcefToUsdTransformForPrim(
        stageId, Context::instance().getGeoreferenceOrigin(), path);
    const auto visible = UsdUtil::isPrimVisible(stageId, path);

    GltfToUsd::createFabricPrims(stageId, tileset.getId(), tileId, visible, ecefToUsdTransform, transform, model);
}
} // namespace

RenderResourcesPreparer::RenderResourcesPreparer(long stageId, const OmniTileset& tileset)
    : _stageId(stageId)
    , _tileset(tileset) {}

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
RenderResourcesPreparer::prepareInLoadThread(
    const CesiumAsync::AsyncSystem& asyncSystem,
    Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
    const glm::dmat4& transform,
    [[maybe_unused]] const std::any& rendererOptions) {
    // TODO: unlit
    // TODO: don't want to do this in main thread
    const auto pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel) {
        return asyncSystem.createResolvedFuture(
            Cesium3DTilesSelection::TileLoadResultAndRenderResources{std::move(tileLoadResult), nullptr});
    }

    return asyncSystem.runInMainThread([this, asyncSystem, transform, tileLoadResult = std::move(tileLoadResult)]() {
        const auto pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
        createFabricPrims(_stageId, _tileset, _tileCount++, transform, *pModel);
        return asyncSystem.createResolvedFuture(
            Cesium3DTilesSelection::TileLoadResultAndRenderResources{std::move(tileLoadResult), nullptr});
    });

    //setActiveOnAllDescendantMeshes(prim, false);
}

void* RenderResourcesPreparer::prepareInMainThread(Cesium3DTilesSelection::Tile& tile, void* pLoadThreadResult) {
    (void)tile;
    (void)pLoadThreadResult;
    return nullptr;
}

void RenderResourcesPreparer::free(
    Cesium3DTilesSelection::Tile& tile,
    void* pLoadThreadResult,
    void* pMainThreadResult) noexcept {
    (void)tile;
    (void)pLoadThreadResult;
    (void)pMainThreadResult;
}

void* RenderResourcesPreparer::prepareRasterInLoadThread(
    CesiumGltf::ImageCesium& image,
    const std::any& rendererOptions) {
    (void)image;
    (void)rendererOptions;
    return nullptr;
}

void* RenderResourcesPreparer::prepareRasterInMainThread(
    Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pLoadThreadResult) {
    (void)rasterTile;
    (void)pLoadThreadResult;
    return nullptr;
}

void RenderResourcesPreparer::freeRaster(
    const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pLoadThreadResult,
    void* pMainThreadResult) noexcept {
    (void)rasterTile;
    (void)pLoadThreadResult;
    (void)pMainThreadResult;
}

void RenderResourcesPreparer::attachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& tile,
    int32_t overlayTextureCoordinateID,
    const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pMainThreadRendererResources,
    const glm::dvec2& translation,
    const glm::dvec2& scale) {
    (void)tile;
    (void)overlayTextureCoordinateID;
    (void)rasterTile;
    (void)pMainThreadRendererResources;
    (void)translation;
    (void)scale;
}

void RenderResourcesPreparer::detachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& tile,
    int32_t overlayTextureCoordinateID,
    const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pMainThreadRendererResources) noexcept {
    (void)tile;
    (void)overlayTextureCoordinateID;
    (void)rasterTile;
    (void)pMainThreadRendererResources;
}

} // namespace cesium::omniverse
