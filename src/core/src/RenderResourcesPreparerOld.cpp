#include "cesium/omniverse/RenderResourcesPreparerOld.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/CoordinateSystem.h"
#include "cesium/omniverse/GltfToUSDOld.h"
#include "cesium/omniverse/UsdUtil.h"

#include <Cesium3DTilesSelection/Tile.h>
#include <Cesium3DTilesSelection/TileID.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdUtils/stitch.h>

#include <atomic>

// TODO: Determine if this is actually needed for anything.
// namespace pxr {
// TF_DEFINE_PRIVATE_TOKENS(_tileTokens, (visibility)(invisible)(visible));
// }

namespace cesium::omniverse {
static std::atomic<std::uint64_t> tileID;

static void setActiveOnAllDescendantMeshes(pxr::UsdPrim prim, bool isActive) {
    for (const auto& descendant : prim.GetAllDescendants()) {
        if (descendant.IsA<pxr::UsdGeomMesh>()) {
            descendant.SetActive(isActive);
        }
    }
}

RenderResourcesPreparerOld::RenderResourcesPreparerOld(const pxr::UsdStageRefPtr& stage_, const pxr::SdfPath& tilesetPath_)
    : stage{stage_}
    , tilesetPath{tilesetPath_} {
    auto xform = pxr::UsdGeomXform::Get(stage, tilesetPath_);
    if (xform) {
        auto prim = xform.GetPrim();
        stage->RemovePrim(prim.GetPath());
    }

    xform = pxr::UsdGeomXform::Define(stage, tilesetPath_);

    pxr::GfMatrix4d currentTransform = glmToGfMatrix(Context::instance().getCoordinateSystem()->getGlobalToLocal());

    tilesetTransform = xform.AddTransformOp();
    tilesetTransform.Set(currentTransform);
}

void RenderResourcesPreparerOld::setTransform(const glm::dmat4& globalToLocal) {
    pxr::GfMatrix4d currentTransform = glmToGfMatrix(globalToLocal);
    tilesetTransform.Set(currentTransform);
}

void RenderResourcesPreparerOld::setVisible(void* renderResources, bool enable) {
    if (renderResources) {
        TileRenderResources* tileRenderResources = reinterpret_cast<TileRenderResources*>(renderResources);
        if (enable != tileRenderResources->enable) {
            if (tileRenderResources->prim) {
                setActiveOnAllDescendantMeshes(tileRenderResources->prim, enable);

                tileRenderResources->enable = enable;
            }
        }
    }
}

CesiumAsync::Future<Cesium3DTilesSelection::TileLoadResultAndRenderResources>
RenderResourcesPreparerOld::prepareInLoadThread(
    const CesiumAsync::AsyncSystem& asyncSystem,
    Cesium3DTilesSelection::TileLoadResult&& tileLoadResult,
    const glm::dmat4& transform,
    [[maybe_unused]] const std::any& rendererOptions) {
    CesiumGltf::Model* pModel = std::get_if<CesiumGltf::Model>(&tileLoadResult.contentKind);
    if (!pModel)
        return asyncSystem.createResolvedFuture(
            Cesium3DTilesSelection::TileLoadResultAndRenderResources{std::move(tileLoadResult), nullptr});

    // It is not possible for multiple threads to simulatenously write to the same stage, but it is safe for different
    // threads to write simultaneously to different stages, which is why we write to an anonymous stage in the load
    // thread and merge with the main stage in the main thread. See
    // https://graphics.pixar.com/usd/release/api/_usd__page__multi_threading.html
    pxr::SdfLayerRefPtr anonLayer = pxr::SdfLayer::CreateAnonymous(".usda");
    pxr::UsdStageRefPtr anonStage = pxr::UsdStage::Open(anonLayer);
    auto prim = GltfToUSD::convertToUSD(
        anonStage,
        tilesetPath.AppendChild(pxr::TfToken(fmt::format("tile_{}", ++tileID))),
        *pModel,
        transform * CesiumGeometry::AxisTransforms::Y_UP_TO_Z_UP);
    setActiveOnAllDescendantMeshes(prim, false);

    return asyncSystem.createResolvedFuture(Cesium3DTilesSelection::TileLoadResultAndRenderResources{
        std::move(tileLoadResult), new TileWorkerRenderResources{std::move(anonLayer), prim.GetPath(), false}});
}

void* RenderResourcesPreparerOld::prepareInMainThread(
    [[maybe_unused]] Cesium3DTilesSelection::Tile& tile,
    void* pLoadThreadResult) {
    if (pLoadThreadResult) {
        std::unique_ptr<TileWorkerRenderResources> workerRenderResources{
            reinterpret_cast<TileWorkerRenderResources*>(pLoadThreadResult)};
        pxr::UsdUtilsStitchLayers(stage->GetRootLayer(), workerRenderResources->layer);
        return new TileRenderResources{stage->GetPrimAtPath(workerRenderResources->primPath), false};
    }

    return nullptr;
}

void RenderResourcesPreparerOld::free(
    [[maybe_unused]] Cesium3DTilesSelection::Tile& tile,
    void* pLoadThreadResult,
    void* pMainThreadResult) noexcept {
    if (pLoadThreadResult) {
        delete reinterpret_cast<TileWorkerRenderResources*>(pLoadThreadResult);
    }

    if (pMainThreadResult) {
        TileRenderResources* tileRenderResources = reinterpret_cast<TileRenderResources*>(pMainThreadResult);
        stage->RemovePrim(tileRenderResources->prim.GetPath());
        delete tileRenderResources;
    }
}

void* RenderResourcesPreparerOld::prepareRasterInLoadThread(
    CesiumGltf::ImageCesium& image,
    [[maybe_unused]] const std::any& rendererOptions) {
    // We don't have access to the tile path so the best we can do is convert the image to a BMP and insert it into the
    // memory asset cache later
    return new RasterRenderResources{GltfToUSD::writeImageToBmp(image)};
}

void* RenderResourcesPreparerOld::prepareRasterInMainThread(
    [[maybe_unused]] Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pLoadThreadResult) {
    if (pLoadThreadResult) {
        // We don't have access to the tile path here either so simply pass along the result of
        // prepareRasterInLoadThread
        return pLoadThreadResult;
    }

    return nullptr;
}

void RenderResourcesPreparerOld::freeRaster(
    [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pLoadThreadResult,
    void* pMainThreadResult) noexcept {
    if (pLoadThreadResult) {
        delete reinterpret_cast<RasterRenderResources*>(pLoadThreadResult);
    }

    if (pMainThreadResult) {
        delete reinterpret_cast<RasterRenderResources*>(pMainThreadResult);
    }
}

void RenderResourcesPreparerOld::attachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& tile,
    [[maybe_unused]] int32_t overlayTextureCoordinateID,
    [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    void* pMainThreadRendererResources,
    const glm::dvec2& translation,
    const glm::dvec2& scale) {
    auto& content = tile.getContent();
    auto pRenderContent = content.getRenderContent();
    if (!pRenderContent) {
        return;
    }

    auto pTileRenderResources = reinterpret_cast<TileRenderResources*>(pRenderContent->getRenderResources());
    if (!pTileRenderResources) {
        return;
    }

    auto pRasterRenderResources = reinterpret_cast<RasterRenderResources*>(pMainThreadRendererResources);
    if (!pRasterRenderResources) {
        return;
    }

    GltfToUSD::insertRasterOverlayTexture(
        pTileRenderResources->prim, std::move(pRasterRenderResources->image), translation, scale);
}

void RenderResourcesPreparerOld::detachRasterInMainThread(
    const Cesium3DTilesSelection::Tile& tile,
    [[maybe_unused]] int32_t overlayTextureCoordinateID,
    [[maybe_unused]] const Cesium3DTilesSelection::RasterOverlayTile& rasterTile,
    [[maybe_unused]] void* pMainThreadRendererResources) noexcept {
    auto& content = tile.getContent();
    auto pRenderContent = content.getRenderContent();
    if (!pRenderContent) {
        return;
    }

    auto pTileRenderResources = reinterpret_cast<TileRenderResources*>(pRenderContent->getRenderResources());
    if (!pTileRenderResources) {
        return;
    }

    GltfToUSD::removeRasterOverlayTexture(pTileRenderResources->prim);
}
} // namespace cesium::omniverse
