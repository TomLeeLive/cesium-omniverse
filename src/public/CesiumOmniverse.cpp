#define CARB_EXPORTS

#include "cesium/omniverse/CesiumOmniverse.h"

#include "CesiumUsdSchemas/data.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/UsdUtil.h"

#include <CesiumGeospatial/Cartographic.h>
#include <carb/PluginUtils.h>
#include <carb/flatcache/FlatCacheUSD.h>
#include <pxr/base/vt/array.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <usdrt/population/IUtils.h>
#include <usdrt/scenegraph/base/gf/range3d.h>
#include <usdrt/scenegraph/base/gf/vec3f.h>
#include <usdrt/scenegraph/usd/rt/xformable.h>
#include <usdrt/scenegraph/usd/sdf/types.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

static const char* SCALE_PRIMVAR_ID = "overlayScale";
static const char* TRANSLATION_PRIMVAR_ID = "overlayTranslation";

// clang-format off
namespace pxr {
TF_DEFINE_PRIVATE_TOKENS(
    _tokens,

    // Tokens used for USD Preview Surface
    // Notes below copied from helloWorld.cpp in Connect Sample 200.0.0
    //
    // Private tokens for building up SdfPaths. We recommend
    // constructing SdfPaths via tokens, as there is a performance
    // cost to constructing them directly via strings (effectively,
    // a table lookup per path element). Similarly, any API which
    // takes a token as input should use a predefined token
    // rather than one created on the fly from a string.
    (a)
    (add)
    ((add_subidentifier, "add(float2,float2)"))
    (b)
    (clamp)
    (coord)
    (data_lookup_uniform_float2)
    (default_value)
    (diffuse_color_constant)
    (diffuseColor)
    (file)
    (i)
    (lookup_color)
    ((matDefault, "default"))
    (mdl)
    (metallic)
    (multiply)
    ((multiply_subidentifier, "multiply(float2,float2)"))
    (name)
    (normal)
    (OmniPBR)
    (out)
    ((PrimStShaderId, "UsdPrimvarReader_float2"))
    (RAW)
    (reflection_roughness_constant)
    (result)
    (rgb)
    (roughness)
    ((scale_primvar, SCALE_PRIMVAR_ID))
    (scale_primvar_reader)
    (sRGB)
    (st)
    (st0)
    (st_0)
    (st_1)
    ((stPrimvarName, "frame:stPrimvarName"))
    (surface)
    (tex)
    (texture_coordinate_2d)
    ((translation_primvar, TRANSLATION_PRIMVAR_ID))
    (translation_primvar_reader)
    (UsdPreviewSurface)
    ((UsdShaderId, "UsdPreviewSurface"))
    (UsdUVTexture)
    (varname)
    (vertex)
    (wrapS)
    (wrapT)
    (wrap_u)
    (wrap_v)
    ((infoIdValueToken, "C:\\users\\slilley\\code\\cesium-omniverse\\extern\\nvidia\\app\\kit\\mdl\\core\\Base\\OmniPBR.mdl"))
    );
}
// clang-format on

namespace cesium::omniverse {

namespace {
pxr::UsdShadeMaterial addMaterialUsd(long stageId) {
    const auto stage = UsdUtil::getUsdStage(stageId);

    // Add material
    const pxr::SdfPath materialPath("/example_material_usd");
    const pxr::TfToken shaderName = pxr::_tokens->OmniPBR;
    const pxr::SdfAssetPath assetPath("OmniPBR.mdl");
    const pxr::TfToken subIdentifier = pxr::_tokens->OmniPBR;
    pxr::UsdShadeMaterial materialUsd = pxr::UsdShadeMaterial::Define(stage, materialPath);
    auto shader = pxr::UsdShadeShader::Define(stage, materialPath.AppendChild(shaderName));
    shader.SetSourceAsset(assetPath, pxr::_tokens->mdl);
    shader.SetSourceAssetSubIdentifier(subIdentifier, pxr::_tokens->mdl);
    shader.CreateIdAttr(pxr::VtValue(shaderName));

    shader.CreateInput(pxr::_tokens->diffuse_color_constant, pxr::SdfValueTypeNames->Vector3f)
        .Set(pxr::GfVec3f(0.0f, 1.0f, 0.0f));

    materialUsd.CreateSurfaceOutput(pxr::_tokens->mdl).ConnectToSource(shader.ConnectableAPI(), pxr::_tokens->out);
    materialUsd.CreateDisplacementOutput(pxr::_tokens->mdl).ConnectToSource(shader.ConnectableAPI(), pxr::_tokens->out);
    materialUsd.CreateVolumeOutput(pxr::_tokens->mdl).ConnectToSource(shader.ConnectableAPI(), pxr::_tokens->out);

    return materialUsd;
}

carb::flatcache::Path addMaterialFabric(long stageId) {
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    const carb::flatcache::Path materialPath("/example_material_fabric");
    const carb::flatcache::Path shaderPath("/example_material_fabric/OmniPBR");
    const carb::flatcache::Path displacementPath("/example_material_fabric/displacement");
    const carb::flatcache::Path surfacePath("/example_material_fabric/surface");

    const auto shaderPathUint64 = carb::flatcache::PathC(shaderPath).path;

    const carb::flatcache::Token terminalsToken("_terminals");
    const carb::flatcache::Token materialToken("Material");

    const carb::flatcache::Token nodePathsToken("_nodePaths");
    const carb::flatcache::Token relationshipsOutputIdToken("_relationships_outputId");
    const carb::flatcache::Token relationshipsInputIdToken("_relationships_inputId");
    const carb::flatcache::Token primvarsToken("primvars");
    const carb::flatcache::Token relationshipsInputNameToken("_relationships_inputName");
    const carb::flatcache::Token relationshipsOutputNameToken("_relationships_outputName");
    const carb::flatcache::Token materialNetworkToken("MaterialNetwork");

    const carb::flatcache::Token infoImplementationSourceToken("info:implementationSource");
    const carb::flatcache::Token infoIdToken("info:id");
    const carb::flatcache::Token infoSourceAssetSubIdentifierToken("info:sourceAsset:subIdentifier");
    const carb::flatcache::Token infoSourceAssetToken("info:sourceAsset");
    const carb::flatcache::Token infoSourceCodeToken("info:sourceCode");
    const carb::flatcache::Token parametersToken("_parameters");
    const carb::flatcache::Token diffuseColorConstantToken("diffuse_color_constant");
    const carb::flatcache::Token shaderToken("Shader");

    const carb::flatcache::Token omniPbrToken("OmniPBR");
    // const carb::flatcache::Token infoIdValueToken("OmniPBR.mdl");

    const carb::flatcache::Token infoIdValueToken(
        "C:\\users\\slilley\\code\\cesium-omniverse\\extern\\nvidia\\app\\kit\\mdl\\core\\Base\\OmniPBR.mdl");

    // Material
    const carb::flatcache::Type terminalsType(
        carb::flatcache::BaseDataType::eUInt64, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type materialType(
        carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);

    // Displacement and surface
    const carb::flatcache::Type nodePathsType(
        carb::flatcache::BaseDataType::eUInt64, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type relationshipsOutputIdType(
        carb::flatcache::BaseDataType::eUInt64, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type relationshipsInputIdType(
        carb::flatcache::BaseDataType::eUInt64, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type primvarsType(
        carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type relationshipsInputNameType(
        carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type relationshipsOutputNameType(
        carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type materialNetworkType(
        carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);

    // Shader
    const carb::flatcache::Type infoImplementationSourceType(
        carb::flatcache::BaseDataType::eToken, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type infoIdType(
        carb::flatcache::BaseDataType::eToken, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type infoSourceAssetSubIdentifierType(
        carb::flatcache::BaseDataType::eToken, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type infoSourceAssetType(
        carb::flatcache::BaseDataType::eAsset, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type infoSourceCodeType(
        carb::flatcache::BaseDataType::eUChar, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type parametersType(
        carb::flatcache::BaseDataType::eToken, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type diffuseColorConstantType(
        carb::flatcache::BaseDataType::eFloat, 3, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type shaderType(
        carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);

    // Material
    stageInProgress.createPrim(materialPath);
    stageInProgress.createAttributes(
        materialPath,
        std::array<carb::flatcache::AttrNameAndType, 2>{
            carb::flatcache::AttrNameAndType{
                terminalsType,
                terminalsToken,
            },
            carb::flatcache::AttrNameAndType{
                materialType,
                materialToken,
            }});

    stageInProgress.setArrayAttributeSize(materialPath, terminalsToken, 2);
    auto terminals = stageInProgress.getArrayAttributeWr<uint64_t>(materialPath, terminalsToken);
    terminals[0] = shaderPathUint64;
    terminals[1] = shaderPathUint64;

    // Displacement
    stageInProgress.createPrim(displacementPath);
    stageInProgress.createAttributes(
        displacementPath,
        std::array<carb::flatcache::AttrNameAndType, 7>{
            carb::flatcache::AttrNameAndType{
                nodePathsType,
                nodePathsToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsOutputIdType,
                relationshipsOutputIdToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsInputIdType,
                relationshipsInputIdToken,
            },
            carb::flatcache::AttrNameAndType{
                primvarsType,
                primvarsToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsInputNameType,
                relationshipsInputNameToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsOutputNameType,
                relationshipsOutputNameToken,
            },
            carb::flatcache::AttrNameAndType{
                materialNetworkType,
                materialNetworkToken,
            }});

    stageInProgress.setArrayAttributeSize(displacementPath, nodePathsToken, 0);
    stageInProgress.setArrayAttributeSize(displacementPath, relationshipsOutputIdToken, 0);
    stageInProgress.setArrayAttributeSize(displacementPath, relationshipsInputIdToken, 0);
    stageInProgress.setArrayAttributeSize(displacementPath, primvarsToken, 0);
    stageInProgress.setArrayAttributeSize(displacementPath, relationshipsInputNameToken, 0);
    stageInProgress.setArrayAttributeSize(displacementPath, relationshipsOutputNameToken, 0);

    // Surface
    stageInProgress.createPrim(surfacePath);
    stageInProgress.createAttributes(
        surfacePath,
        std::array<carb::flatcache::AttrNameAndType, 7>{
            carb::flatcache::AttrNameAndType{
                nodePathsType,
                nodePathsToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsOutputIdType,
                relationshipsOutputIdToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsInputIdType,
                relationshipsInputIdToken,
            },
            carb::flatcache::AttrNameAndType{
                primvarsType,
                primvarsToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsInputNameType,
                relationshipsInputNameToken,
            },
            carb::flatcache::AttrNameAndType{
                relationshipsOutputNameType,
                relationshipsOutputNameToken,
            },
            carb::flatcache::AttrNameAndType{
                materialNetworkType,
                materialNetworkToken,
            }});

    stageInProgress.setArrayAttributeSize(surfacePath, nodePathsToken, 0);
    stageInProgress.setArrayAttributeSize(surfacePath, relationshipsOutputIdToken, 0);
    stageInProgress.setArrayAttributeSize(surfacePath, relationshipsInputIdToken, 0);
    stageInProgress.setArrayAttributeSize(surfacePath, primvarsToken, 0);
    stageInProgress.setArrayAttributeSize(surfacePath, relationshipsInputNameToken, 0);
    stageInProgress.setArrayAttributeSize(surfacePath, relationshipsOutputNameToken, 0);

    // Shader
    stageInProgress.createPrim(shaderPath);
    stageInProgress.createAttributes(
        shaderPath,
        std::array<carb::flatcache::AttrNameAndType, 5>{// carb::flatcache::AttrNameAndType{
                                                        //     infoImplementationSourceType,
                                                        //     infoImplementationSourceToken,
                                                        // },
                                                        carb::flatcache::AttrNameAndType{
                                                            infoIdType,
                                                            infoIdToken,
                                                        },
                                                        carb::flatcache::AttrNameAndType{
                                                            infoSourceAssetSubIdentifierType,
                                                            infoSourceAssetSubIdentifierToken,
                                                        },
                                                        // carb::flatcache::AttrNameAndType{
                                                        //     infoSourceAssetType,
                                                        //     infoSourceAssetToken,
                                                        // },
                                                        // carb::flatcache::AttrNameAndType{
                                                        //     infoSourceCodeType,
                                                        //     infoSourceCodeToken,
                                                        // },
                                                        carb::flatcache::AttrNameAndType{
                                                            parametersType,
                                                            parametersToken,
                                                        },
                                                        carb::flatcache::AttrNameAndType{
                                                            diffuseColorConstantType,
                                                            diffuseColorConstantToken,
                                                        },
                                                        carb::flatcache::AttrNameAndType{
                                                            shaderType,
                                                            shaderToken,
                                                        }});

    auto infoId = stageInProgress.getAttributeWr<carb::flatcache::Token>(shaderPath, infoIdToken);
    *infoId = infoIdValueToken;

    auto infoSourceAssetSubIdentifier =
        stageInProgress.getAttributeWr<carb::flatcache::Token>(shaderPath, infoSourceAssetSubIdentifierToken);
    *infoSourceAssetSubIdentifier = omniPbrToken;

    stageInProgress.setArrayAttributeSize(shaderPath, parametersToken, 1);
    auto parameters = stageInProgress.getArrayAttributeWr<carb::flatcache::Token>(shaderPath, parametersToken);
    parameters[0] = diffuseColorConstantToken;

    auto diffuseColorConstant = stageInProgress.getAttributeWr<usdrt::GfVec3f>(shaderPath, diffuseColorConstantToken);
    *diffuseColorConstant = usdrt::GfVec3f(0.0, 1.0, 0.0);

    // // TODO: infoSourceAssetToken is an "asset" type
    // auto inputSourceAsset = stageInProgress.getAttributeWr<std::array<uint64_t, 8>>(shaderPath, infoSourceAssetToken);
    // *inputSourceAsset = std::array<uint64_t, 8>{0, 0, 0, 0, 0, 0, 0, 0};

    return materialPath;
}

} // namespace
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

        const auto materialUsd = addMaterialUsd(stageId);

        pxr::UsdShadeMaterialBindingAPI usdMaterialBinding(prim);
        usdMaterialBinding.Bind(materialUsd);
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
        const carb::flatcache::Token stToken("primvars:st");
        const carb::flatcache::Token meshToken("Mesh");
        const carb::flatcache::Token constantToken("constant");
        const carb::flatcache::Token vertexToken("vertex");
        const carb::flatcache::Token worldPositionToken("_worldPosition");
        const carb::flatcache::Token worldOrientationToken("_worldOrientation");
        const carb::flatcache::Token worldScaleToken("_worldScale");
        const carb::flatcache::Token localMatrixToken("_localMatrix");
        const carb::flatcache::Token materialIdToken("materialId");

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

        const carb::flatcache::Type stType(
            carb::flatcache::BaseDataType::eFloat, 2, 1, carb::flatcache::AttributeRole::eTexCoord);

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

        const carb::flatcache::Type materialIdType(
            carb::flatcache::BaseDataType::eToken, 1, 0, carb::flatcache::AttributeRole::eNone);

        stageInProgress.createPrim(primPath);
        stageInProgress.createAttributes(
            primPath,
            std::array<carb::flatcache::AttrNameAndType, 15>{
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
                    stType,
                    stToken,
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
                carb::flatcache::AttrNameAndType{
                    materialIdType,
                    materialIdToken,
                }});

        stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, 12);
        stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, 36);
        stageInProgress.setArrayAttributeSize(primPath, pointsToken, 24);
        stageInProgress.setArrayAttributeSize(primPath, primvarsToken, 2);
        stageInProgress.setArrayAttributeSize(primPath, primvarInterpolationsToken, 2);
        stageInProgress.setArrayAttributeSize(primPath, displayColorToken, 1);
        stageInProgress.setArrayAttributeSize(primPath, stToken, 24);

        const auto faceVertexCounts = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
        const auto faceVertexIndices = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
        const auto points = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
        const auto worldExtent = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);
        const auto visibility = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
        const auto primvars = stageInProgress.getArrayAttributeWr<carb::flatcache::Token>(primPath, primvarsToken);
        const auto primvarInterpolations =
            stageInProgress.getArrayAttributeWr<carb::flatcache::Token>(primPath, primvarInterpolationsToken);
        const auto displayColor = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, displayColorToken);
        const auto st = stageInProgress.getArrayAttributeWr<usdrt::GfVec2f>(primPath, stToken);
        const auto worldPosition = stageInProgress.getAttributeWr<usdrt::GfVec3d>(primPath, worldPositionToken);
        const auto worldOrientation = stageInProgress.getAttributeWr<usdrt::GfQuatf>(primPath, worldOrientationToken);
        const auto worldScale = stageInProgress.getAttributeWr<usdrt::GfVec3f>(primPath, worldScaleToken);
        const auto localMatrix = stageInProgress.getAttributeWr<usdrt::GfMatrix4d>(primPath, localMatrixToken);
        const auto materialId = stageInProgress.getAttributeWr<carb::flatcache::Token>(primPath, materialIdToken);

        faceVertexCounts[0] = 3;
        faceVertexCounts[1] = 3;
        faceVertexCounts[2] = 3;
        faceVertexCounts[3] = 3;
        faceVertexCounts[4] = 3;
        faceVertexCounts[5] = 3;
        faceVertexCounts[6] = 3;
        faceVertexCounts[7] = 3;
        faceVertexCounts[8] = 3;
        faceVertexCounts[9] = 3;
        faceVertexCounts[10] = 3;
        faceVertexCounts[11] = 3;

        faceVertexIndices[0] = 14;
        faceVertexIndices[1] = 7;
        faceVertexIndices[2] = 1;
        faceVertexIndices[3] = 6;
        faceVertexIndices[4] = 23;
        faceVertexIndices[5] = 10;
        faceVertexIndices[6] = 18;
        faceVertexIndices[7] = 15;
        faceVertexIndices[8] = 21;
        faceVertexIndices[9] = 3;
        faceVertexIndices[10] = 22;
        faceVertexIndices[11] = 16;
        faceVertexIndices[12] = 2;
        faceVertexIndices[13] = 11;
        faceVertexIndices[14] = 5;
        faceVertexIndices[15] = 13;
        faceVertexIndices[16] = 4;
        faceVertexIndices[17] = 17;
        faceVertexIndices[18] = 14;
        faceVertexIndices[19] = 20;
        faceVertexIndices[20] = 7;
        faceVertexIndices[21] = 6;
        faceVertexIndices[22] = 19;
        faceVertexIndices[23] = 23;
        faceVertexIndices[24] = 18;
        faceVertexIndices[25] = 12;
        faceVertexIndices[26] = 15;
        faceVertexIndices[27] = 3;
        faceVertexIndices[28] = 9;
        faceVertexIndices[29] = 22;
        faceVertexIndices[30] = 2;
        faceVertexIndices[31] = 8;
        faceVertexIndices[32] = 11;
        faceVertexIndices[33] = 13;
        faceVertexIndices[34] = 0;
        faceVertexIndices[35] = 4;

        points[0] = usdrt::GfVec3f(1.0, 1.0, -1.0);
        points[1] = usdrt::GfVec3f(1.0, 1.0, -1.0);
        points[2] = usdrt::GfVec3f(1.0, 1.0, -1.0);
        points[3] = usdrt::GfVec3f(1.0, -1.0, -1.0);
        points[4] = usdrt::GfVec3f(1.0, -1.0, -1.0);
        points[5] = usdrt::GfVec3f(1.0, -1.0, -1.0);
        points[6] = usdrt::GfVec3f(1.0, 1.0, 1.0);
        points[7] = usdrt::GfVec3f(1.0, 1.0, 1.0);
        points[8] = usdrt::GfVec3f(1.0, 1.0, 1.0);
        points[9] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[10] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[11] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[12] = usdrt::GfVec3f(-1.0, 1.0, -1.0);
        points[13] = usdrt::GfVec3f(-1.0, 1.0, -1.0);
        points[14] = usdrt::GfVec3f(-1.0, 1.0, -1.0);
        points[15] = usdrt::GfVec3f(-1.0, -1.0, -1.0);
        points[16] = usdrt::GfVec3f(-1.0, -1.0, -1.0);
        points[17] = usdrt::GfVec3f(-1.0, -1.0, -1.0);
        points[18] = usdrt::GfVec3f(-1.0, 1.0, 1.0);
        points[19] = usdrt::GfVec3f(-1.0, 1.0, 1.0);
        points[20] = usdrt::GfVec3f(-1.0, 1.0, 1.0);
        points[21] = usdrt::GfVec3f(-1.0, -1.0, 1.0);
        points[22] = usdrt::GfVec3f(-1.0, -1.0, 1.0);
        points[23] = usdrt::GfVec3f(-1.0, -1.0, 1.0);

        // normals[0] = usdrt::GfVec3f(0.0, 0.0, -1.0);
        // normals[1] = usdrt::GfVec3f(0.0, 1.0, 0.0);
        // normals[2] = usdrt::GfVec3f(1.0, 0.0, 0.0);
        // normals[3] = usdrt::GfVec3f(0.0, -1.0, 0.0);
        // normals[4] = usdrt::GfVec3f(0.0, 0.0, -1.0);
        // normals[5] = usdrt::GfVec3f(1.0, 0.0, 0.0);
        // normals[6] = usdrt::GfVec3f(0.0, 0.0, 1.0);
        // normals[7] = usdrt::GfVec3f(0.0, 1.0, 0.0);
        // normals[8] = usdrt::GfVec3f(1.0, 0.0, 0.0);
        // normals[9] = usdrt::GfVec3f(0.0, -1.0, 0.0);
        // normals[10] = usdrt::GfVec3f(0.0, 0.0, 1.0);
        // normals[11] = usdrt::GfVec3f(1.0, 0.0, 0.0);
        // normals[12] = usdrt::GfVec3f(-1.0, 0.0, 0.0);
        // normals[13] = usdrt::GfVec3f(0.0, 0.0, -1.0);
        // normals[14] = usdrt::GfVec3f(0.0, 1.0, 0.0);
        // normals[15] = usdrt::GfVec3f(-1.0, 0.0, 0.0);
        // normals[16] = usdrt::GfVec3f(0.0, -1.0, 0.0);
        // normals[17] = usdrt::GfVec3f(0.0, 0.0, -1.0);
        // normals[18] = usdrt::GfVec3f(-1.0, 0.0, 0.0);
        // normals[19] = usdrt::GfVec3f(0.0, 0.0, 1.0);
        // normals[20] = usdrt::GfVec3f(0.0, 1.0, 0.0);
        // normals[21] = usdrt::GfVec3f(-1.0, 0.0, 0.0);
        // normals[22] = usdrt::GfVec3f(0.0, -1.0, 0.0);
        // normals[23] = usdrt::GfVec3f(0.0, 0.0, 1.0);

        st[0] = usdrt::GfVec2f(1.0, 0.0);
        st[1] = usdrt::GfVec2f(0.0, 1.0);
        st[2] = usdrt::GfVec2f(1.0, 0.0);
        st[3] = usdrt::GfVec2f(0.0, 0.0);
        st[4] = usdrt::GfVec2f(0.0, 0.0);
        st[5] = usdrt::GfVec2f(1.0, 1.0);
        st[6] = usdrt::GfVec2f(1.0, 0.0);
        st[7] = usdrt::GfVec2f(0.0, 0.0);
        st[8] = usdrt::GfVec2f(0.0, 0.0);
        st[9] = usdrt::GfVec2f(0.0, 1.0);
        st[10] = usdrt::GfVec2f(1.0, 1.0);
        st[11] = usdrt::GfVec2f(0.0, 1.0);
        st[12] = usdrt::GfVec2f(1.0, 0.0);
        st[13] = usdrt::GfVec2f(1.0, 1.0);
        st[14] = usdrt::GfVec2f(1.0, 1.0);
        st[15] = usdrt::GfVec2f(0.0, 0.0);
        st[16] = usdrt::GfVec2f(1.0, 0.0);
        st[17] = usdrt::GfVec2f(0.0, 1.0);
        st[18] = usdrt::GfVec2f(1.0, 1.0);
        st[19] = usdrt::GfVec2f(0.0, 0.0);
        st[20] = usdrt::GfVec2f(1.0, 0.0);
        st[21] = usdrt::GfVec2f(0.0, 1.0);
        st[22] = usdrt::GfVec2f(1.0, 1.0);
        st[23] = usdrt::GfVec2f(0.0, 1.0);

        worldExtent->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
        worldExtent->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));

        *visibility = true;
        primvars[0] = displayColorToken;
        primvars[1] = stToken;
        primvarInterpolations[0] = constantToken;
        primvarInterpolations[1] = vertexToken;
        displayColor[0] = usdrt::GfVec3f(0.0, 0.0, 1.0);
        *worldPosition = usdrt::GfVec3d(0.0, 0.0, 0.0);
        *worldOrientation = usdrt::GfQuatf(1.0);
        *worldScale = usdrt::GfVec3f(1.0);
        *localMatrix = UsdUtil::glmToUsdrtMatrix(glm::dmat4(1.0));

        addMaterialFabric(stageId);

        const carb::flatcache::Token materialToken("/example_material_fabric");
        *materialId = materialToken;
        // (void)materialId;

        // // Set the material
        // const auto materialUsd = addMaterialUsd(stageId);
        // const auto materialPathUsd = materialUsd.GetPath();
        // const auto materialPathUsdrt = usdrt::SdfPath(materialUsd.GetPath().GetString());
        // const auto materialPathFc = carb::flatcache::asInt(materialPathUsd);

        //const auto stageUsdrt = UsdUtil::getUsdrtStage(stageId);
        //stageUsdrt->GetPrimAtPath(materialPathUsdrt);
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
