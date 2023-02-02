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
#include <usdrt/population/IUtils.h>
#include <usdrt/scenegraph/base/gf/range3d.h>
#include <usdrt/scenegraph/base/gf/vec3f.h>
#include <usdrt/scenegraph/usd/rt/xformable.h>
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

        const usdrt::SdfPath primPath("/example_prim_usdrt");

        const usdrt::TfToken faceVertexCountsToken("faceVertexCounts");
        const usdrt::TfToken faceVertexIndicesToken("faceVertexIndices");
        const usdrt::TfToken pointsToken("points");
        const usdrt::TfToken worldExtentToken("_worldExtent");
        const usdrt::TfToken visibilityToken("visibility");
        const usdrt::TfToken primvarsToken("primvars");
        const usdrt::TfToken primvarInterpolationsToken("primvarInterpolations");
        const usdrt::TfToken displayColorToken("primvars:displayColor");
        const usdrt::TfToken meshToken("Mesh");
        const usdrt::TfToken constantToken("constant");

        const usdrt::UsdPrim prim = stage->DefinePrim(primPath, meshToken);

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

        prim.CreateAttribute(faceVertexCountsToken, usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexCounts);
        prim.CreateAttribute(faceVertexIndicesToken, usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexIndices);
        prim.CreateAttribute(pointsToken, usdrt::SdfValueTypeNames->Point3fArray, false).Set(points);
        prim.CreateAttribute(displayColorToken, usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
        prim.CreateAttribute(worldExtentToken, usdrt::SdfValueTypeNames->Range3d, false).Set(range);
        prim.CreateAttribute(visibilityToken, usdrt::SdfValueTypeNames->Bool, false).Set(true);

        // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
        // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
        // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
        const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(displayColorToken)};
        const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(constantToken)};

        prim.CreateAttribute(primvarsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
        prim.CreateAttribute(primvarInterpolationsToken, usdrt::SdfValueTypeNames->TokenArray, false)
            .Set(primvarInterp);

        const auto xform = usdrt::RtXformable(prim);
        xform.CreateLocalMatrixAttr(UsdUtil::glmToUsdrtMatrix(glm::dmat4(1.0)));
        xform.CreateWorldPositionAttr(usdrt::GfVec3d(0.0, 0.0, 0.0));
        xform.CreateWorldOrientationAttr(usdrt::GfQuatf(1.0));
        xform.CreateWorldScaleAttr(usdrt::GfVec3f(1.0));

        const usdrt::TfToken materialIdToken("materialId");
        prim.CreateAttribute(materialIdToken, usdrt::SdfValueTypeNames->token.AddTarget(usdrt::SdfPath("/World/Looks/OmniPBR"));
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

        const carb::flatcache::Path primPath("/example_prim_fabric");

        const carb::flatcache::Token faceVertexCountsToken("faceVertexCounts");
        const carb::flatcache::Token faceVertexIndicesToken("faceVertexIndices");
        const carb::flatcache::Token pointsToken("points");
        const carb::flatcache::Token worldExtentToken("_worldExtent");
        const carb::flatcache::Token visibilityToken("visibility");
        const carb::flatcache::Token primvarsToken("primvars");
        const carb::flatcache::Token primvarInterpolationsToken("primvarInterpolations");
        const carb::flatcache::Token displayColorToken("primvars:displayColor");
        const carb::flatcache::Token meshToken("Mesh");
        const carb::flatcache::Token constantToken("constant");
        const carb::flatcache::Token worldPositionToken("_worldPosition");
        const carb::flatcache::Token worldOrientationToken("_worldOrientation");
        const carb::flatcache::Token worldScaleToken("_worldScale");
        const carb::flatcache::Token localMatrixToken("_localMatrix");

        const carb::flatcache::Type faceVertexCountsType(
            carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type faceVertexIndicesType(
            carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type pointsType(
            carb::flatcache::BaseDataType::eFloat, 3, 1, carb::flatcache::AttributeRole::ePosition);

        const carb::flatcache::Type worldExtentType(
            carb::flatcache::BaseDataType::eDouble, 6, 0, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type visibilityType(
            carb::flatcache::BaseDataType::eBool, 1, 0, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type primvarsType(
            carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type primvarInterpolationsType(
            carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type displayColorType(
            carb::flatcache::BaseDataType::eFloat, 3, 1, carb::flatcache::AttributeRole::eColor);

        const carb::flatcache::Type meshType(
            carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);

        const carb::flatcache::Type worldPositionType(
            carb::flatcache::BaseDataType::eDouble, 3, 0, carb::flatcache::AttributeRole::eNone);

        const carb::flatcache::Type worldOrientationType(
            carb::flatcache::BaseDataType::eFloat, 4, 0, carb::flatcache::AttributeRole::eQuaternion);

        const carb::flatcache::Type worldScaleType(
            carb::flatcache::BaseDataType::eFloat, 3, 0, carb::flatcache::AttributeRole::eVector);

        const carb::flatcache::Type localMatrixType(
            carb::flatcache::BaseDataType::eDouble, 16, 0, carb::flatcache::AttributeRole::eMatrix);

        stageInProgress.createPrim(primPath);
        stageInProgress.createAttributes(
            primPath,
            std::array<carb::flatcache::AttrNameAndType, 13>{
                carb::flatcache::AttrNameAndType{
                    faceVertexCountsType,
                    faceVertexCountsToken,
                },
                carb::flatcache::AttrNameAndType{
                    faceVertexIndicesType,
                    faceVertexIndicesToken,
                },
                carb::flatcache::AttrNameAndType{
                    pointsType,
                    pointsToken,
                },
                carb::flatcache::AttrNameAndType{
                    worldExtentType,
                    worldExtentToken,
                },
                carb::flatcache::AttrNameAndType{
                    visibilityType,
                    visibilityToken,
                },
                carb::flatcache::AttrNameAndType{
                    primvarsType,
                    primvarsToken,
                },
                carb::flatcache::AttrNameAndType{
                    primvarInterpolationsType,
                    primvarInterpolationsToken,
                },
                carb::flatcache::AttrNameAndType{
                    displayColorType,
                    displayColorToken,
                },
                carb::flatcache::AttrNameAndType{
                    meshType,
                    meshToken,
                },
                carb::flatcache::AttrNameAndType{
                    worldPositionType,
                    worldPositionToken,
                },
                carb::flatcache::AttrNameAndType{
                    worldOrientationType,
                    worldOrientationToken,
                },
                carb::flatcache::AttrNameAndType{
                    worldScaleType,
                    worldScaleToken,
                },
                carb::flatcache::AttrNameAndType{
                    localMatrixType,
                    localMatrixToken,
                },
            });

        stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, 6);
        stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, 24);
        stageInProgress.setArrayAttributeSize(primPath, pointsToken, 8);
        stageInProgress.setArrayAttributeSize(primPath, primvarsToken, 1);
        stageInProgress.setArrayAttributeSize(primPath, primvarInterpolationsToken, 1);
        stageInProgress.setArrayAttributeSize(primPath, displayColorToken, 1);

        const auto faceVertexCounts = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
        const auto faceVertexIndices = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
        const auto points = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
        const auto worldExtent = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);
        const auto visibility = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
        const auto primvars = stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarsToken);
        const auto primvarInterpolations =
            stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarInterpolationsToken);
        const auto displayColor = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, displayColorToken);
        const auto worldPosition = stageInProgress.getAttributeWr<usdrt::GfVec3d>(primPath, worldPositionToken);
        const auto worldOrientation = stageInProgress.getAttributeWr<usdrt::GfQuatf>(primPath, worldOrientationToken);
        const auto worldScale = stageInProgress.getAttributeWr<usdrt::GfVec3f>(primPath, worldScaleToken);
        const auto localMatrix = stageInProgress.getAttributeWr<usdrt::GfMatrix4d>(primPath, localMatrixToken);

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
        points[6] = usdrt::GfVec3f(1.0, 1.0, -1.0);
        points[7] = usdrt::GfVec3f(1.0, 1.0, 1.0);

        worldExtent->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
        worldExtent->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));

        *visibility = true;
        primvars[0] = carb::flatcache::TokenC(displayColorToken);
        primvarInterpolations[0] = carb::flatcache::TokenC(constantToken);
        displayColor[0] = usdrt::GfVec3f(1.0, 0.0, 0.0);
        *worldPosition = usdrt::GfVec3d(0.0, 0.0, 0.0);
        *worldOrientation = usdrt::GfQuatf(1.0);
        *worldScale = usdrt::GfVec3f(1.0);
        *localMatrix = UsdUtil::glmToUsdrtMatrix(glm::dmat4(1.0));
    }

    void showCubeUsdrt(long stageId) noexcept override {
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        carb::flatcache::Path primPath("/example_prim_usdrt");
        carb::flatcache::Token visibilityToken("visibility");

        auto token = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
        *token = true;
    }

    void hideCubeUsdrt(long stageId) noexcept override {
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        carb::flatcache::Path primPath("/example_prim_usdrt");
        carb::flatcache::Token visibilityToken("visibility");

        auto token = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
        *token = false;
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

    void populateUsdStageIntoFabric(long stageId) noexcept override {
        const auto stage = UsdUtil::getUsdrtStage(stageId);
        stage->Traverse();
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
