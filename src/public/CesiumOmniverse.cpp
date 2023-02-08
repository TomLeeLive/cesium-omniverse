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

namespace cesium::omniverse {

namespace {

carb::flatcache::Path addMaterialFabricCopy(long stageId, const char* path) {
    // Create a fabric material that copies an existing material
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    const carb::flatcache::Path materialCopyPath("/World/Looks/OmniPBR_Green");
    const carb::flatcache::Path materialCopyShaderPath("/World/Looks/OmniPBR_Green/Shader");
    const carb::flatcache::Path materialCopyDisplacementPath("/World/Looks/OmniPBR_Green/displacement");
    const carb::flatcache::Path materialCopySurfacePath("/World/Looks/OmniPBR_Green/surface");

    const carb::flatcache::Path materialPath(path);
    const carb::flatcache::Path shaderPath(fmt::format("{}/OmniPBR", path).c_str());
    const carb::flatcache::Path displacementPath(fmt::format("{}/displacement", path).c_str());
    const carb::flatcache::Path surfacePath(fmt::format("{}/surface", path).c_str());

    stageInProgress.createPrim(materialPath);
    stageInProgress.createPrim(shaderPath);
    stageInProgress.createPrim(displacementPath);
    stageInProgress.createPrim(surfacePath);

    stageInProgress.copyAttributes(materialCopyPath, materialPath);
    stageInProgress.copyAttributes(materialCopyShaderPath, shaderPath);
    stageInProgress.copyAttributes(materialCopyDisplacementPath, displacementPath);
    stageInProgress.copyAttributes(materialCopySurfacePath, surfacePath);

    return materialPath;
}

carb::flatcache::Path addMaterialFabricCopy2(long stageId, const char* path) {
    // Create a fabric material that uses an existing material's shader
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    const carb::flatcache::Path materialPath(path);

    const carb::flatcache::Path shaderPath("/World/Looks/OmniPBR_Green/Shader");
    const auto shaderPathUint64 = carb::flatcache::PathC(shaderPath).path;

    const carb::flatcache::Token terminalsToken("_terminals");
    const carb::flatcache::Token materialToken("Material");

    const carb::flatcache::Type terminalsType(
        carb::flatcache::BaseDataType::eUInt64, 1, 1, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type materialType(
        carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);

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

    return materialPath;
}

carb::flatcache::Path addMaterialFabric(long stageId, const char* path) {
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    const carb::flatcache::Path materialPath(path);
    const carb::flatcache::Path shaderPath(fmt::format("{}/OmniPBR", path).c_str());
    const carb::flatcache::Path displacementPath(fmt::format("{}/displacement", path).c_str());
    const carb::flatcache::Path surfacePath(fmt::format("{}/surface", path).c_str());

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
    const carb::flatcache::Token infoIdValueToken(
        "C:\\users\\slilley\\code\\cesium-omniverse\\extern\\nvidia\\app\\kit\\mdl\\core\\Base\\OmniPBR.mdl");
    // const carb::flatcache::Token infoIdValueToken("OmniPBR.mdl");

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
        carb::flatcache::BaseDataType::eFloat, 3, 0, carb::flatcache::AttributeRole::eColor);

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

    // // Displacement
    // stageInProgress.createPrim(displacementPath);
    // stageInProgress.createAttributes(
    //     displacementPath,
    //     std::array<carb::flatcache::AttrNameAndType, 7>{
    //         carb::flatcache::AttrNameAndType{
    //             nodePathsType,
    //             nodePathsToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsOutputIdType,
    //             relationshipsOutputIdToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsInputIdType,
    //             relationshipsInputIdToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             primvarsType,
    //             primvarsToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsInputNameType,
    //             relationshipsInputNameToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsOutputNameType,
    //             relationshipsOutputNameToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             materialNetworkType,
    //             materialNetworkToken,
    //         }});

    // stageInProgress.setArrayAttributeSize(displacementPath, nodePathsToken, 0);
    // stageInProgress.setArrayAttributeSize(displacementPath, relationshipsOutputIdToken, 0);
    // stageInProgress.setArrayAttributeSize(displacementPath, relationshipsInputIdToken, 0);
    // stageInProgress.setArrayAttributeSize(displacementPath, primvarsToken, 0);
    // stageInProgress.setArrayAttributeSize(displacementPath, relationshipsInputNameToken, 0);
    // stageInProgress.setArrayAttributeSize(displacementPath, relationshipsOutputNameToken, 0);

    // // Surface
    // stageInProgress.createPrim(surfacePath);
    // stageInProgress.createAttributes(
    //     surfacePath,
    //     std::array<carb::flatcache::AttrNameAndType, 7>{
    //         carb::flatcache::AttrNameAndType{
    //             nodePathsType,
    //             nodePathsToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsOutputIdType,
    //             relationshipsOutputIdToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsInputIdType,
    //             relationshipsInputIdToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             primvarsType,
    //             primvarsToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsInputNameType,
    //             relationshipsInputNameToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             relationshipsOutputNameType,
    //             relationshipsOutputNameToken,
    //         },
    //         carb::flatcache::AttrNameAndType{
    //             materialNetworkType,
    //             materialNetworkToken,
    //         }});

    // stageInProgress.setArrayAttributeSize(surfacePath, nodePathsToken, 0);
    // stageInProgress.setArrayAttributeSize(surfacePath, relationshipsOutputIdToken, 0);
    // stageInProgress.setArrayAttributeSize(surfacePath, relationshipsInputIdToken, 0);
    // stageInProgress.setArrayAttributeSize(surfacePath, primvarsToken, 0);
    // stageInProgress.setArrayAttributeSize(surfacePath, relationshipsInputNameToken, 0);
    // stageInProgress.setArrayAttributeSize(surfacePath, relationshipsOutputNameToken, 0);

    // Shader
    stageInProgress.createPrim(shaderPath);
    stageInProgress.createAttributes(
        shaderPath,
        std::array<carb::flatcache::AttrNameAndType, 5>{
            carb::flatcache::AttrNameAndType{
                infoIdType,
                infoIdToken,
            },
            carb::flatcache::AttrNameAndType{
                infoSourceAssetSubIdentifierType,
                infoSourceAssetSubIdentifierToken,
            },
            carb::flatcache::AttrNameAndType{
                parametersType,
                parametersToken,
            },
            carb::flatcache::AttrNameAndType{
                diffuseColorConstantType,
                diffuseColorConstantToken,
            },
            // carb::flatcache::AttrNameAndType{
            //     infoSourceAssetType,
            //     infoSourceAssetToken,
            // },
            // carb::flatcache::AttrNameAndType{
            //     infoSourceCodeType,
            //     infoSourceCodeToken,
            // },
            // carb::flatcache::AttrNameAndType{
            //     infoImplementationSourceType,
            //     infoImplementationSourceToken,
            // },
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

    return materialPath;
}

carb::flatcache::Path addCubeFabric(long stageId, float translation, const char* path) {
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    const carb::flatcache::Path primPath(path);

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

    // TODO: make this not so verbose
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

    points[0] = usdrt::GfVec3f(50.0f, 50.0f + translation, -50.0f);
    points[1] = usdrt::GfVec3f(50.0f, 50.0f + translation, -50.0f);
    points[2] = usdrt::GfVec3f(50.0f, 50.0f + translation, -50.0f);
    points[3] = usdrt::GfVec3f(50.0f, -50.0f + translation, -50.0f);
    points[4] = usdrt::GfVec3f(50.0f, -50.0f + translation, -50.0f);
    points[5] = usdrt::GfVec3f(50.0f, -50.0f + translation, -50.0f);
    points[6] = usdrt::GfVec3f(50.0f, 50.0f + translation, 50.0f);
    points[7] = usdrt::GfVec3f(50.0f, 50.0f + translation, 50.0f);
    points[8] = usdrt::GfVec3f(50.0f, 50.0f + translation, 50.0f);
    points[9] = usdrt::GfVec3f(50.0f, -50.0f + translation, 50.0f);
    points[10] = usdrt::GfVec3f(50.0f, -50.0f + translation, 50.0f);
    points[11] = usdrt::GfVec3f(50.0f, -50.0f + translation, 50.0f);
    points[12] = usdrt::GfVec3f(-50.0f, 50.0f + translation, -50.0f);
    points[13] = usdrt::GfVec3f(-50.0f, 50.0f + translation, -50.0f);
    points[14] = usdrt::GfVec3f(-50.0f, 50.0f + translation, -50.0f);
    points[15] = usdrt::GfVec3f(-50.0f, -50.0f + translation, -50.0f);
    points[16] = usdrt::GfVec3f(-50.0f, -50.0f + translation, -50.0f);
    points[17] = usdrt::GfVec3f(-50.0f, -50.0f + translation, -50.0f);
    points[18] = usdrt::GfVec3f(-50.0f, 50.0f + translation, 50.0f);
    points[19] = usdrt::GfVec3f(-50.0f, 50.0f + translation, 50.0f);
    points[20] = usdrt::GfVec3f(-50.0f, 50.0f + translation, 50.0f);
    points[21] = usdrt::GfVec3f(-50.0f, -50.0f + translation, 50.0f);
    points[22] = usdrt::GfVec3f(-50.0f, -50.0f + translation, 50.0f);
    points[23] = usdrt::GfVec3f(-50.0f, -50.0f + translation, 50.0f);

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

    worldExtent->SetMin(usdrt::GfVec3d(-50.0f, -50.0f + translation, -50.0f));
    worldExtent->SetMax(usdrt::GfVec3d(50.0f, 50.0f + translation, 50.0f));

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

    (void)materialId;

    return primPath;
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

    void addCubeFabricExistingMaterial(long stageId) noexcept override {
        // Create a mesh directly in Fabric and assign an existing material
        // Make sure to open yellow_green.usda first
        // This works
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        const auto fabricPrimPath = addCubeFabric(stageId, 0.0f, "/prim_fabric_existing_material");

        const carb::flatcache::Token greenMaterialPrimPath("/World/Looks/OmniPBR_Green");
        const carb::flatcache::Token materialIdToken("materialId");

        const auto materialId = stageInProgress.getAttributeWr<carb::flatcache::Token>(fabricPrimPath, materialIdToken);
        *materialId = greenMaterialPrimPath;
    }

    void addCubeFabricNewMaterial(long stageId) noexcept override {
        // Create a mesh and material directly in Fabric
        // This doesn't work: red material - something went wrong
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        const auto fabricPrimPath = addCubeFabric(stageId, 150.0f, "/prim_fabric_new_material");
        const auto fabricMaterialPath = addMaterialFabric(stageId, "/material_fabric");

        const carb::flatcache::Token materialIdToken("materialId");

        const auto materialId = stageInProgress.getAttributeWr<carb::flatcache::Token>(fabricPrimPath, materialIdToken);
        *materialId = carb::flatcache::Token(fabricMaterialPath.getText());
    }

    void addCubeFabricCopyMaterial(long stageId) noexcept override {
        // Create a mesh and material directly in Fabric. The material is copied from an existing material.
        // Make sure to open yellow_green.usda first
        // This doesn't work: red material - something went wrong
        auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

        const auto fabricPrimPath = addCubeFabric(stageId, 300.0f, "/prim_fabric_copy_material");
        const auto fabricMaterialPath = addMaterialFabricCopy(stageId, "/material_fabric_copy");

        const carb::flatcache::Token materialIdToken("materialId");

        const auto materialId = stageInProgress.getAttributeWr<carb::flatcache::Token>(fabricPrimPath, materialIdToken);
        *materialId = carb::flatcache::Token(fabricMaterialPath.getText());
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
