#define CARB_EXPORTS

#include "cesium/omniverse/CesiumOmniverse.h"

#include "CesiumUsdSchemas/data.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/UsdUtil.h"

#include <CesiumGeospatial/Cartographic.h>
#include <carb/PluginUtils.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <usdrt/scenegraph/base/gf/range3d.h>
#include <usdrt/scenegraph/base/gf/vec3f.h>
#include <usdrt/scenegraph/usd/sdf/types.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

namespace cesium::omniverse {

class CesiumOmniversePlugin : public ICesiumOmniverseInterface {
  protected:
    void init(const char* cesiumExtensionLocation) noexcept override {
        Context::instance().init(cesiumExtensionLocation);
    }

    void destroy() noexcept {
        Context::instance().destroy();
    }

    void addCesiumData(long stageId, const char* ionToken) noexcept override {
        const auto stage = UsdUtil::getUsdStage(stageId);
        pxr::UsdPrim cesiumDataPrim = stage->DefinePrim(pxr::SdfPath("/Cesium"));
        pxr::CesiumData cesiumData(cesiumDataPrim);
        auto ionTokenAttr = cesiumData.CreateIonTokenAttr(pxr::VtValue(""));

        if (strlen(ionToken) != 0) {
            ionTokenAttr.Set(ionToken);
        }
    }

    int addTilesetUrl(long stageId, const char* url) noexcept override {
        return Context::instance().addTilesetUrl(stageId, url);
    }

    int addTilesetIon(long stageId, int64_t ionId, const char* ionToken) noexcept override {
        return Context::instance().addTilesetIon(stageId, ionId, ionToken);
    }

    void removeTileset(int tilesetId) noexcept override {
        Context::instance().removeTileset(tilesetId);
    }

    void addIonRasterOverlay(int tilesetId, const char* name, int64_t ionId, const char* ionToken) noexcept override {
        Context::instance().addIonRasterOverlay(tilesetId, name, ionId, ionToken);
    }

    void updateFrame(
        long stageId,
        const pxr::GfMatrix4d& viewMatrix,
        const pxr::GfMatrix4d& projMatrix,
        double width,
        double height) noexcept override {
        const auto viewMatrixGlm = UsdUtil::usdToGlmMatrix(viewMatrix);
        const auto projMatrixGlm = UsdUtil::usdToGlmMatrix(projMatrix);
        Context::instance().updateFrame(stageId, viewMatrixGlm, projMatrixGlm, width, height);
    }

    void setGeoreferenceOrigin(double longitude, double latitude, double height) noexcept override {
        CesiumGeospatial::Cartographic cartographic(glm::radians(longitude), glm::radians(latitude), height);
        Context::instance().setGeoreferenceOrigin(cartographic);
    }

    void addCubeUsdrt(long stageId) noexcept override {
        const auto stage = UsdUtil::getUsdrtStage(stageId);

        // Create a cube prim.
        const usdrt::SdfPath primPath("/example_prim_usdrt");
        const usdrt::UsdPrim prim = stage->DefinePrim(primPath, usdrt::TfToken("Mesh"));

        const usdrt::VtArray<int> faceVertexCounts = {4, 4, 4, 4, 4, 4};
        const usdrt::VtArray<int> faceVertexIndices = {
            0, 1, 3, 2, 2, 3, 7, 6, 6, 7, 5, 4, 4, 5, 1, 0, 2, 6, 4, 0, 7, 3, 1, 5,
        };

        const usdrt::VtArray<usdrt::GfVec3f> points = {
            usdrt::GfVec3f(-1.0, -1.0, -1.0),
            usdrt::GfVec3f(-1.0, -1.0, 1.0),
            usdrt::GfVec3f(-1.0, 1.0, -1.0),
            usdrt::GfVec3f(-1.0, 1.0, 1.0),
            usdrt::GfVec3f(1.0, -1.0, -1.0),
            usdrt::GfVec3f(1.0, -1.0, 1.0),
            usdrt::GfVec3f(1.0, 1.0, -1.0),
            usdrt::GfVec3f(1.0, 1.0, 1.0)};

        usdrt::VtArray<usdrt::GfVec3f> displayColor = {usdrt::GfVec3f(1.0, 0.0, 0.0)};

        usdrt::GfRange3d range(usdrt::GfVec3d(-1.0, -1.0, -1.0), usdrt::GfVec3d(1.0, 1.0, 1.0));

        // clang-format off
        prim.CreateAttribute(usdrt::TfToken("faceVertexCounts"), usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexCounts);
        prim.CreateAttribute(usdrt::TfToken("faceVertexIndices"), usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexIndices);
        prim.CreateAttribute(usdrt::TfToken("points"), usdrt::SdfValueTypeNames->Point3fArray, false).Set(points);
        prim.CreateAttribute(usdrt::TfToken("primvars:displayColor"), usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
        prim.CreateAttribute(usdrt::TfToken("_worldExtent"), usdrt::SdfValueTypeNames->Range3d, false).Set(range);

        // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
        // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
        // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
        const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(usdrt::TfToken("primvars:displayColor"))};
        const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(usdrt::TfToken("constant"))};

        prim.CreateAttribute(usdrt::TfToken("primvars"), usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
        prim.CreateAttribute(usdrt::TfToken("primvarInterpolations"), usdrt::SdfValueTypeNames->TokenArray, false).Set(primvarInterp);
        // clang-format on
    }

    void addCubeUsd(long stageId) noexcept override {
        const auto stage = UsdUtil::getUsdStage(stageId);

        // Create a cube prim.
        const pxr::SdfPath primPath("/example_prim_usd");
        const pxr::UsdPrim prim = stage->DefinePrim(primPath, pxr::TfToken("Mesh"));

        const pxr::VtArray<int> faceVertexCounts = {4, 4, 4, 4, 4, 4};
        const pxr::VtArray<int> faceVertexIndices = {0, 1, 3, 2, 2, 3, 7, 6, 6, 7, 5, 4,
                                                     4, 5, 1, 0, 2, 6, 4, 0, 7, 3, 1, 5};

        const pxr::VtArray<pxr::GfVec3f> points = {
            pxr::GfVec3f(-1.0, -1.0, -1.0),
            pxr::GfVec3f(-1.0, -1.0, 1.0),
            pxr::GfVec3f(-1.0, 1.0, -1.0),
            pxr::GfVec3f(-1.0, 1.0, 1.0),
            pxr::GfVec3f(1.0, -1.0, -1.0),
            pxr::GfVec3f(1.0, -1.0, 1.0),
            pxr::GfVec3f(1.0, 1.0, -1.0),
            pxr::GfVec3f(1.0, 1.0, 1.0)};

        pxr::VtArray<pxr::GfVec3f> displayColor = {pxr::GfVec3f(1.0, 0.0, 0.0)};

        // clang-format off
        prim.CreateAttribute(pxr::TfToken("faceVertexCounts"), pxr::SdfValueTypeNames->IntArray).Set(faceVertexCounts);
        prim.CreateAttribute(pxr::TfToken("faceVertexIndices"), pxr::SdfValueTypeNames->IntArray).Set(faceVertexIndices);
        prim.CreateAttribute(pxr::TfToken("points"), pxr::SdfValueTypeNames->Point3fArray).Set(points);
        prim.CreateAttribute(pxr::TfToken("primvars:displayColor"), pxr::SdfValueTypeNames->Color3fArray).Set(displayColor);
        // clang-format on
    }

    void addCubeFabric(long stageId) noexcept override {
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        carb::flatcache::Path primPath("/example_prim_fabric");
        carb::flatcache::Token faceVertexCountsToken("faceVertexCounts");
        carb::flatcache::Token faceVertexIndicesToken("faceVertexIndices");
        carb::flatcache::Token pointsToken("points");
        carb::flatcache::Token worldExtentToken("_worldExtent");

        stageInProgress.createPrim(primPath);
        stageInProgress.createAttribute(
            primPath,
            faceVertexCountsToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone));
        stageInProgress.createAttribute(
            primPath,
            faceVertexIndicesToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone));

        stageInProgress.createAttribute(
            primPath,
            pointsToken,
            carb::flatcache::Type(
                carb::flatcache::BaseDataType::eFloat, 3, 1, carb::flatcache::AttributeRole::ePosition));

        stageInProgress.createAttribute(
            primPath,
            worldExtentToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eDouble, 6, 0, carb::flatcache::AttributeRole::eNone));

        stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, 6);
        stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, 24);
        stageInProgress.setArrayAttributeSize(primPath, pointsToken, 8);

        auto faceVertexCounts = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
        auto faceVertexIndices = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
        auto points = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
        auto worldExtent = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);

        faceVertexCounts[0] = 4;
        faceVertexCounts[1] = 4;
        faceVertexCounts[2] = 4;
        faceVertexCounts[3] = 4;
        faceVertexCounts[4] = 4;
        faceVertexCounts[5] = 4;

        faceVertexIndices[0] = 0;
        faceVertexIndices[1] = 1;
        faceVertexIndices[2] = 3;
        faceVertexIndices[3] = 2;
        faceVertexIndices[4] = 2;
        faceVertexIndices[5] = 3;
        faceVertexIndices[6] = 7;
        faceVertexIndices[7] = 6;
        faceVertexIndices[8] = 6;
        faceVertexIndices[9] = 7;
        faceVertexIndices[10] = 5;
        faceVertexIndices[11] = 4;
        faceVertexIndices[12] = 4;
        faceVertexIndices[13] = 5;
        faceVertexIndices[14] = 1;
        faceVertexIndices[15] = 0;
        faceVertexIndices[16] = 2;
        faceVertexIndices[17] = 6;
        faceVertexIndices[18] = 4;
        faceVertexIndices[19] = 0;
        faceVertexIndices[20] = 7;
        faceVertexIndices[21] = 3;
        faceVertexIndices[22] = 1;
        faceVertexIndices[23] = 5;

        points[0] = usdrt::GfVec3f(-1.0, -1.0, -1.0);
        points[1] = usdrt::GfVec3f(-1.0, -1.0, 1.0);
        points[2] = usdrt::GfVec3f(-1.0, 1.0, -1.0);
        points[3] = usdrt::GfVec3f(-1.0, 1.0, 1.0);
        points[4] = usdrt::GfVec3f(1.0, -1.0, -1.0);
        points[5] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[6] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[7] = usdrt::GfVec3f(1.0, 1.0, 1.0);

        worldExtent->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
        worldExtent->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));
    }

    void removeCubeUsdrt(long stageId) noexcept override {
        const auto stage = UsdUtil::getUsdrtStage(stageId);
        const auto primPath = usdrt::SdfPath("/example_prim_usdrt");

        stage->RemovePrim(primPath);

        // Prims removed from Fabric need special handling for their removal to be reflected in the Hydra render index
        // This workaround may not be needed in future Kit versions
        usdrt::UsdAttribute deletedPrimsAttribute =
            stage->GetAttributeAtPath(usdrt::SdfPath("/TempChangeTracking._deletedPrims"));

        if (!deletedPrimsAttribute.IsValid()) {
            return;
        }

        usdrt::VtArray<uint64_t> result;
        result.push_back(carb::flatcache::PathC(primPath).path);
        deletedPrimsAttribute.Set(result);
    }

    std::string printUsdrtStage(long stageId) noexcept override {
        return UsdUtil::printUsdrtStage(stageId);
    }

    std::string printFabricStage(long stageId) noexcept override {
        return UsdUtil::printFabricStage(stageId);
    }
};
} // namespace cesium::omniverse

const struct carb::PluginImplDesc pluginImplDesc = {
    "cesium.omniverse.plugin",
    "Cesium Omniverse Carbonite Plugin.",
    "Cesium",
    carb::PluginHotReload::eDisabled,
    "dev"};

CARB_PLUGIN_IMPL(pluginImplDesc, cesium::omniverse::CesiumOmniversePlugin)

void fillInterface([[maybe_unused]] cesium::omniverse::CesiumOmniversePlugin& iface) {}
