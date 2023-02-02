#include "cesium/omniverse/GltfToUSD.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/InMemoryAssetResolver.h"
#include "cesium/omniverse/UsdUtil.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <Cesium3DTilesSelection/GltfUtilities.h>
#include <CesiumGeometry/AxisTransforms.h>
#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/Model.h>
#include <CesiumGltfReader/GltfReader.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <pxr/usd/sdf/assetPath.h>
#include <spdlog/fmt/fmt.h>
#include <usdrt/gf/range.h>
#include <usdrt/gf/vec.h>
#include <usdrt/scenegraph/usd/rt/xformable.h>
#include <usdrt/scenegraph/usd/sdf/types.h>

#include <regex>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <numeric>
#include <optional>

namespace cesium::omniverse::GltfToUsd {

namespace {

glm::dmat4 unpackMatrix(const std::vector<double>& values) {
    return glm::dmat4(
        values[0],
        values[1],
        values[2],
        values[3],
        values[4],
        values[5],
        values[6],
        values[7],
        values[8],
        values[9],
        values[10],
        values[11],
        values[12],
        values[13],
        values[14],
        values[15]);
}

glm::dvec3 unpackVec3(const std::vector<double>& values) {
    return glm::dvec3(values[0], values[1], values[2]);
}

glm::dquat unpackQuat(const std::vector<double>& values) {
    return glm::dquat(values[0], values[1], values[2], values[3]);
}

glm::dmat4 getNodeMatrix(const CesiumGltf::Node& node) {
    glm::dmat4 nodeMatrix(1.0);

    if (node.matrix.size() == 16) {
        nodeMatrix = unpackMatrix(node.matrix);
    } else {
        glm::dmat4 translation(1.0);
        glm::dmat4 rotation(1.0);
        glm::dmat4 scale(1.0);

        if (node.scale.size() == 3) {
            scale = glm::scale(scale, unpackVec3(node.scale));
        }

        if (node.rotation.size() == 4) {
            rotation = glm::toMat4(unpackQuat(node.rotation));
        }

        if (node.translation.size() == 3) {
            translation = glm::translate(translation, unpackVec3(node.translation));
        }

        nodeMatrix = translation * rotation * scale;
    }

    return nodeMatrix;
}

usdrt::VtArray<usdrt::GfVec3f>
getPrimitivePositions(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive) {
    auto positionAttribute = primitive.attributes.find("POSITION");
    if (positionAttribute == primitive.attributes.end()) {
        return {};
    }

    auto positionAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, positionAttribute->second);
    if (!positionAccessor) {
        return {};
    }

    auto positions = CesiumGltf::AccessorView<glm::vec3>(model, *positionAccessor);
    if (positions.status() != CesiumGltf::AccessorViewStatus::Valid) {
        return {};
    }

    // TODO: a memcpy should work just as well
    usdrt::VtArray<usdrt::GfVec3f> usdPositions;
    usdPositions.reserve(static_cast<uint64_t>(positions.size()));
    for (auto i = 0; i < positions.size(); i++) {
        const auto& position = positions[i];
        usdPositions.push_back(usdrt::GfVec3f(position.x, position.y, position.z));
    }

    return usdPositions;
}

std::optional<usdrt::GfRange3d>
getPrimitiveExtent(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive) {
    auto positionAttribute = primitive.attributes.find("POSITION");
    if (positionAttribute == primitive.attributes.end()) {
        return std::nullopt;
    }

    auto positionAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, positionAttribute->second);
    if (!positionAccessor) {
        return std::nullopt;
    }

    const auto& min = positionAccessor->min;
    const auto& max = positionAccessor->max;

    if (min.size() != 3 || max.size() != 3) {
        return std::nullopt;
    }

    return usdrt::GfRange3d(usdrt::GfVec3d(min[0], min[1], min[2]), usdrt::GfVec3d(max[0], max[1], max[2]));
}

template <typename IndexType>
usdrt::VtArray<int> createIndices(
    const CesiumGltf::MeshPrimitive& primitive,
    const CesiumGltf::AccessorView<IndexType>& indicesAccessorView) {
    if (indicesAccessorView.status() != CesiumGltf::AccessorViewStatus::Valid) {
        return {};
    }

    if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLES) {
        if (indicesAccessorView.size() % 3 != 0) {
            return {};
        }

        usdrt::VtArray<int> indices;
        indices.reserve(static_cast<uint64_t>(indicesAccessorView.size()));
        for (auto i = 0; i < indicesAccessorView.size(); i++) {
            indices.push_back(static_cast<int>(indicesAccessorView[i]));
        }

        return indices;
    }

    if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLE_STRIP) {
        if (indicesAccessorView.size() <= 2) {
            return {};
        }

        usdrt::VtArray<int> indices;
        indices.reserve(static_cast<uint64_t>(indicesAccessorView.size() - 2) * 3);
        for (auto i = 0; i < indicesAccessorView.size() - 2; i++) {
            if (i % 2) {
                indices.push_back(static_cast<int>(indicesAccessorView[i]));
                indices.push_back(static_cast<int>(indicesAccessorView[i + 2]));
                indices.push_back(static_cast<int>(indicesAccessorView[i + 1]));
            } else {
                indices.push_back(static_cast<int>(indicesAccessorView[i]));
                indices.push_back(static_cast<int>(indicesAccessorView[i + 1]));
                indices.push_back(static_cast<int>(indicesAccessorView[i + 2]));
            }
        }

        return indices;
    }

    if (primitive.mode == CesiumGltf::MeshPrimitive::Mode::TRIANGLE_FAN) {
        if (indicesAccessorView.size() <= 2) {
            return {};
        }

        usdrt::VtArray<int> indices;
        indices.reserve(static_cast<uint64_t>(indicesAccessorView.size() - 2) * 3);
        for (auto i = 0; i < indicesAccessorView.size() - 2; i++) {
            indices.push_back(static_cast<int>(indicesAccessorView[0]));
            indices.push_back(static_cast<int>(indicesAccessorView[i + 1]));
            indices.push_back(static_cast<int>(indicesAccessorView[i + 2]));
        }

        return indices;
    }

    return {};
}

usdrt::VtArray<int> getPrimitiveIndices(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const usdrt::VtArray<usdrt::GfVec3f>& positions) {
    const CesiumGltf::Accessor* indicesAccessor =
        model.getSafe<CesiumGltf::Accessor>(&model.accessors, primitive.indices);
    if (!indicesAccessor) {
        usdrt::VtArray<int> indices;
        indices.reserve(positions.size());
        for (auto i = 0; i < positions.size(); i++) {
            indices.push_back(i);
        }
        return indices;
    }

    if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_BYTE) {
        CesiumGltf::AccessorView<std::uint8_t> view{model, *indicesAccessor};
        return createIndices(primitive, view);
    } else if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_SHORT) {
        CesiumGltf::AccessorView<std::uint16_t> view{model, *indicesAccessor};
        return createIndices(primitive, view);
    } else if (indicesAccessor->componentType == CesiumGltf::AccessorSpec::ComponentType::UNSIGNED_INT) {
        CesiumGltf::AccessorView<std::uint32_t> view{model, *indicesAccessor};
        return createIndices(primitive, view);
    }

    return {};
}

usdrt::VtArray<usdrt::GfVec3f> getPrimitiveNormals(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const usdrt::VtArray<usdrt::GfVec3f>& positions,
    const usdrt::VtArray<int>& indices) {
    // get normal view
    auto normalAttribute = primitive.attributes.find("NORMAL");
    if (normalAttribute != primitive.attributes.end()) {
        auto normalsView = CesiumGltf::AccessorView<glm::vec3>(model, normalAttribute->second);
        if (normalsView.status() == CesiumGltf::AccessorViewStatus::Valid) {
            usdrt::VtArray<usdrt::GfVec3f> normalsUsd;
            normalsUsd.reserve(static_cast<uint64_t>(normalsView.size()));
            for (auto i = 0; i < normalsView.size(); ++i) {
                const glm::vec3& n = normalsView[i];
                normalsUsd.push_back(usdrt::GfVec3f(n.x, n.y, n.z));
            }

            return normalsUsd;
        }
    }

    // Generate smooth normals
    usdrt::VtArray<usdrt::GfVec3f> normalsUsd;
    normalsUsd.reserve(static_cast<uint64_t>(positions.size()));

    for (auto i = 0; i < positions.size(); i++) {
        normalsUsd.push_back(usdrt::GfVec3f(0.0));
    }

    for (auto i = 0; i < indices.size(); i += 3) {
        auto idx0 = indices[i];
        auto idx1 = indices[i + 1];
        auto idx2 = indices[i + 2];

        const usdrt::GfVec3f& p0 = positions[idx0];
        const usdrt::GfVec3f& p1 = positions[idx1];
        const usdrt::GfVec3f& p2 = positions[idx2];
        usdrt::GfVec3f n = usdrt::GfCross(p1 - p0, p2 - p0);
        n.Normalize();

        normalsUsd[idx0] += n;
        normalsUsd[idx1] += n;
        normalsUsd[idx2] += n;
    }

    for (auto& n : normalsUsd) {
        n.Normalize();
    }

    return normalsUsd;
}

CesiumGltf::AccessorView<glm::vec2> getUVsAccessorView(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const std::string& semantic,
    uint64_t setIndex) {
    auto uvAttribute = primitive.attributes.find(fmt::format("{}_{}", semantic, setIndex));
    if (uvAttribute == primitive.attributes.end()) {
        return CesiumGltf::AccessorView<glm::vec2>();
    }

    auto uvAccessor = model.getSafe<CesiumGltf::Accessor>(&model.accessors, uvAttribute->second);
    if (!uvAccessor) {
        return CesiumGltf::AccessorView<glm::vec2>();
    }

    return CesiumGltf::AccessorView<glm::vec2>(model, *uvAccessor);
}

usdrt::VtArray<usdrt::GfVec2f> getUVs(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const std::string& semantic,
    uint64_t setIndex,
    bool flipUVs) {
    auto uvs = getUVsAccessorView(model, primitive, semantic, setIndex);
    if (uvs.status() != CesiumGltf::AccessorViewStatus::Valid) {
        return {};
    }

    usdrt::VtArray<usdrt::GfVec2f> usdUVs;
    usdUVs.reserve(static_cast<uint64_t>(uvs.size()));
    for (auto i = 0; i < uvs.size(); ++i) {
        glm::vec2 vert = uvs[i];

        if (flipUVs) {
            vert.y = 1.0f - vert.y;
        }

        usdUVs.push_back(usdrt::GfVec2f(vert.x, vert.y));
    }

    return usdUVs;
}

usdrt::VtArray<usdrt::GfVec2f>
getPrimitiveUVs(const CesiumGltf::Model& model, const CesiumGltf::MeshPrimitive& primitive, uint64_t setIndex) {
    return getUVs(model, primitive, "TEXCOORD", setIndex, true);
}

usdrt::VtArray<int> getFaceVertexCounts(const usdrt::VtArray<int>& indices) {
    const auto faceCount = indices.size() / 3;
    usdrt::VtArray<int> faceVertexCounts;
    faceVertexCounts.reserve(faceCount);

    for (auto i = 0; i < faceCount; i++) {
        faceVertexCounts.push_back(3);
    }

    return faceVertexCounts;
}

std::vector<std::byte> writeImageToBmp(const CesiumGltf::ImageCesium& image) {
    std::vector<std::byte> writeData;
    stbi_write_bmp_to_func(
        [](void* context, void* data, int size) {
            auto& write = *reinterpret_cast<std::vector<std::byte>*>(context);
            const auto bdata = reinterpret_cast<std::byte*>(data);
            write.insert(write.end(), bdata, bdata + size);
        },
        &writeData,
        image.width,
        image.height,
        image.channels,
        image.pixelData.data());

    return writeData;
}

pxr::SdfAssetPath convertTextureToUsd(
    const CesiumGltf::Model& model,
    const CesiumGltf::Texture& texture,
    const std::string& textureName) {
    const auto imageId = static_cast<uint64_t>(texture.source);
    const auto& image = model.images[imageId];

    auto inMemoryAsset = std::make_shared<pxr::InMemoryAsset>(writeImageToBmp(image.cesium));
    auto& ctx = pxr::InMemoryAssetContext::instance();
    ctx.assets.insert({textureName, std::move(inMemoryAsset)});

    const auto memCesiumPath = Context::instance().getMemCesiumPath().generic_string();
    const auto assetPath = fmt::format("{}[{}]", memCesiumPath, textureName);

    return pxr::SdfAssetPath(assetPath);
}

void convertPrimitiveToUsdrt(
    usdrt::UsdStageRefPtr stage,
    int tilesetId,
    bool visible,
    const std::string& primName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive) {

    const auto positions = getPrimitivePositions(model, primitive);
    const auto indices = getPrimitiveIndices(model, primitive, positions);
    const auto normals = getPrimitiveNormals(model, primitive, positions, indices);
    const auto st0 = getPrimitiveUVs(model, primitive, 0);
    const auto st1 = getPrimitiveUVs(model, primitive, 1);
    const auto worldExtent = getPrimitiveExtent(model, primitive);
    const auto faceVertexCounts = getFaceVertexCounts(indices);

    if (positions.empty() || indices.empty() || normals.empty() || !worldExtent.has_value()) {
        return;
    }

    const usdrt::SdfPath primPath(fmt::format("/{}", primName));

    const usdrt::VtArray<usdrt::GfVec3f> displayColor = {usdrt::GfVec3f(1.0, 0.0, 0.0)};

    const auto localToEcefTransform = gltfToEcefTransform * nodeTransform;
    const auto localToUsdTransform = ecefToUsdTransform * localToEcefTransform;
    const auto [worldPosition, worldOrientation, worldScale] = UsdUtil::glmToUsdrtMatrixDecomposed(localToUsdTransform);

    // TODO: precreate tokens
    const usdrt::TfToken faceVertexCountsToken("faceVertexCounts");
    const usdrt::TfToken faceVertexIndicesToken("faceVertexIndices");
    const usdrt::TfToken pointsToken("points");
    const usdrt::TfToken worldExtentToken("_worldExtent");
    const usdrt::TfToken visibilityToken("visibility");
    const usdrt::TfToken constantToken("constant");
    const usdrt::TfToken primvarsToken("primvars");
    const usdrt::TfToken primvarInterpolationsToken("primvarInterpolations");
    const usdrt::TfToken displayColorToken("primvars:displayColor");
    const usdrt::TfToken meshToken("Mesh");
    const usdrt::TfToken tilesetIdToken("_tilesetId");

    const usdrt::UsdPrim prim = stage->DefinePrim(primPath, meshToken);

    // localToUsdTransform = transform glTF local position to USD world position
    //
    // 1. Start with glTF local position
    // 2. Apply glTF scene graph
    // 3. Apply y-up to z-up to bring into 3D Tiles coordinate system
    // 4. Apply tile transform. Now positions are in ECEF coordinates.
    // 5. Apply Fixed Frame to ENU based on the georeference origin
    // 6. Apply USD axis conversion
    // 7. Apply USD unit conversion
    // 8. Apply Tileset prim transform
    // 9. End with USD world position
    //
    // Here we save localToEcefTransform in the _localMatrix attribute so we can access it later if anything changes
    // after step 4 and we need to recompute _worldPosition, _worldOrientation, _worldScale
    // Note that _worldPosition, _worldOrientation, _worldScale override _localMatrix
    // See http://omniverse-docs.s3-website-us-east-1.amazonaws.com/usdrt/5.0.0/docs/omnihydra_xforms.html
    const auto xform = usdrt::RtXformable(prim);
    xform.CreateLocalMatrixAttr(UsdUtil::glmToUsdrtMatrix(localToEcefTransform));
    xform.CreateWorldPositionAttr(worldPosition);
    xform.CreateWorldOrientationAttr(worldOrientation);
    xform.CreateWorldScaleAttr(worldScale);

    // TODO: normals
    prim.CreateAttribute(faceVertexCountsToken, usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexCounts);
    prim.CreateAttribute(faceVertexIndicesToken, usdrt::SdfValueTypeNames->IntArray, false).Set(indices);
    prim.CreateAttribute(pointsToken, usdrt::SdfValueTypeNames->Point3fArray, false).Set(positions);
    prim.CreateAttribute(worldExtentToken, usdrt::SdfValueTypeNames->Range3d, false).Set(worldExtent.value());
    prim.CreateAttribute(visibilityToken, usdrt::SdfValueTypeNames->Bool, false).Set(visible);
    prim.CreateAttribute(displayColorToken, usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
    prim.CreateAttribute(tilesetIdToken, usdrt::SdfValueTypeNames->Int, true).Set(tilesetId);

    // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
    // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
    // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
    const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(displayColorToken)};
    const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(constantToken)};

    prim.CreateAttribute(primvarsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
    prim.CreateAttribute(primvarInterpolationsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvarInterp);
}

void convertPrimitiveToFabric(
    carb::flatcache::StageInProgress& stageInProgress,
    int tilesetId,
    bool visible,
    const std::string& primName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive) {

    auto positions = getPrimitivePositions(model, primitive);
    auto indices = getPrimitiveIndices(model, primitive, positions);
    auto normals = getPrimitiveNormals(model, primitive, positions, indices);
    auto st0 = getPrimitiveUVs(model, primitive, 0);
    auto st1 = getPrimitiveUVs(model, primitive, 1);
    auto worldExtent = getPrimitiveExtent(model, primitive);
    auto faceVertexCounts = getFaceVertexCounts(indices);

    if (positions.empty() || indices.empty() || normals.empty() || !worldExtent.has_value()) {
        return;
    }

    const carb::flatcache::Path primPath(fmt::format("/{}", primName).c_str());

    const auto displayColor = usdrt::GfVec3f(1.0, 0.0, 0.0);

    auto localToEcefTransform = gltfToEcefTransform * nodeTransform;
    auto localToUsdTransform = ecefToUsdTransform * localToEcefTransform;
    auto [worldPosition, worldOrientation, worldScale] = UsdUtil::glmToUsdrtMatrixDecomposed(localToUsdTransform);

    // TODO: precreate tokens
    const carb::flatcache::Token faceVertexCountsToken("faceVertexCounts");
    const carb::flatcache::Token faceVertexIndicesToken("faceVertexIndices");
    const carb::flatcache::Token pointsToken("points");
    const carb::flatcache::Token worldExtentToken("_worldExtent");
    const carb::flatcache::Token visibilityToken("visibility");
    const carb::flatcache::Token constantToken("constant");
    const carb::flatcache::Token primvarsToken("primvars");
    const carb::flatcache::Token primvarInterpolationsToken("primvarInterpolations");
    const carb::flatcache::Token displayColorToken("primvars:displayColor");
    const carb::flatcache::Token meshToken("Mesh");
    const carb::flatcache::Token tilesetIdToken("_tilesetId");
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

    const carb::flatcache::Type tilesetIdType(
        carb::flatcache::BaseDataType::eInt, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type worldPositionType(
        carb::flatcache::BaseDataType::eDouble, 3, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type worldOrientationType(
        carb::flatcache::BaseDataType::eFloat, 4, 0, carb::flatcache::AttributeRole::eQuaternion);

    const carb::flatcache::Type worldScaleType(
        carb::flatcache::BaseDataType::eFloat, 3, 0, carb::flatcache::AttributeRole::eVector);

    const carb::flatcache::Type localMatrixType(
        carb::flatcache::BaseDataType::eDouble, 16, 0, carb::flatcache::AttributeRole::eMatrix);

    stageInProgress.createPrim(primPath);

    stageInProgress.createPrim(primPath);
    stageInProgress.createAttributes(
        primPath,
        std::array<carb::flatcache::AttrNameAndType, 14>{
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
                tilesetIdType,
                tilesetIdToken,
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

    stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, faceVertexCounts.size());
    stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, indices.size());
    stageInProgress.setArrayAttributeSize(primPath, pointsToken, positions.size());
    stageInProgress.setArrayAttributeSize(primPath, primvarsToken, 1);
    stageInProgress.setArrayAttributeSize(primPath, primvarInterpolationsToken, 1);
    stageInProgress.setArrayAttributeSize(primPath, displayColorToken, 1);

    auto faceVertexCountsFabric = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
    auto faceVertexIndicesFabric = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
    auto pointsFabric = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
    auto worldExtentFabric = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);
    auto visibilityFabric = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
    auto primvarsFabric = stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarsToken);
    auto primvarInterpolationsFabric =
        stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarInterpolationsToken);
    auto displayColorFabric = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, displayColorToken);
    auto tilesetIdFabric = stageInProgress.getAttributeWr<int>(primPath, tilesetIdToken);
    auto worldPositionFabric = stageInProgress.getAttributeWr<usdrt::GfVec3d>(primPath, worldPositionToken);
    auto worldOrientationFabric = stageInProgress.getAttributeWr<usdrt::GfQuatf>(primPath, worldOrientationToken);
    auto worldScaleFabric = stageInProgress.getAttributeWr<usdrt::GfVec3f>(primPath, worldScaleToken);
    auto localMatrixFabric = stageInProgress.getAttributeWr<usdrt::GfMatrix4d>(primPath, localMatrixToken);

    std::copy(faceVertexCounts.begin(), faceVertexCounts.end(), faceVertexCountsFabric.begin());
    std::copy(indices.begin(), indices.end(), faceVertexIndicesFabric.begin());
    std::copy(positions.begin(), positions.end(), pointsFabric.begin());

    worldExtentFabric->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
    worldExtentFabric->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));

    *visibilityFabric = visible;
    primvarsFabric[0] = carb::flatcache::TokenC(displayColorToken);
    primvarInterpolationsFabric[0] = carb::flatcache::TokenC(constantToken);
    displayColorFabric[0] = displayColor;
    *tilesetIdFabric = tilesetId;
    *worldPositionFabric = worldPosition;
    *worldOrientationFabric = worldOrientation;
    *worldScaleFabric = worldScale;
    *localMatrixFabric = UsdUtil::glmToUsdrtMatrix(localToEcefTransform);
}

std::string sanitizeAssetPath(const std::string& assetPath) {
    const std::regex regex("[\\W]+");
    const std::string replace = "_";

    return std::regex_replace(assetPath, regex, replace);
}

std::string getTexturePrefix(const CesiumGltf::Model& model, const std::string& tileName) {
    // Texture asset paths need to be uniquely identifiable in order for Omniverse texture caching to function correctly.
    // If the tile url doesn't exist (which is unlikely), fall back to the tile name. Just be aware that the tile name
    // is not guaranteed to be the same across app invocations and caching may break.
    std::string texturePrefix = tileName;
    auto urlIt = model.extras.find("Cesium3DTiles_TileUrl");
    if (urlIt != model.extras.end()) {
        texturePrefix = urlIt->second.getStringOrDefault(tileName);
    }

    // Replace path separators and other special characters with underscores.
    // As of Kit 104.1, the texture cache only includes the name after the last path separator in the cache key.
    return sanitizeAssetPath(texturePrefix);
}

} // namespace

void createFabricPrims(
    long stageId,
    int tilesetId,
    uint64_t tileId,
    bool visible,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& tileTransform,
    const CesiumGltf::Model& model) {
    const auto stage = UsdUtil::getUsdrtStage(stageId);
    auto stageInProgress = UsdUtil::getFabricStageInProgress(stageId);

    auto gltfToEcefTransform = Cesium3DTilesSelection::GltfUtilities::applyRtcCenter(model, tileTransform);
    gltfToEcefTransform = Cesium3DTilesSelection::GltfUtilities::applyGltfUpAxisTransform(model, gltfToEcefTransform);

    const auto tileName = fmt::format("tileset_{}_tile_{}", tilesetId, tileId);
    const auto texturePrefix = getTexturePrefix(model, tileName);

    std::vector<pxr::SdfAssetPath> textureUsdPaths;
    textureUsdPaths.reserve(model.textures.size());
    for (auto i = 0; i < model.textures.size(); ++i) {
        const auto textureName = fmt::format("{}_texture_{}", texturePrefix, i);
        textureUsdPaths.emplace_back(convertTextureToUsd(model, model.textures[i], textureName));
    }

    uint64_t primitiveId = 0;

    model.forEachPrimitiveInScene(
        -1,
        [&stage,
         &stageInProgress,
         &tileName,
         tilesetId,
         visible,
         &primitiveId,
         &ecefToUsdTransform,
         &gltfToEcefTransform](
            const CesiumGltf::Model& gltf,
            [[maybe_unused]] const CesiumGltf::Node& node,
            [[maybe_unused]] const CesiumGltf::Mesh& mesh,
            const CesiumGltf::MeshPrimitive& primitive,
            const glm::dmat4& transform) {
            const auto primName = fmt::format("{}_primitive_{}", tileName, primitiveId++);
            convertPrimitiveToUsdrt(
                stage,
                tilesetId,
                visible,
                primName,
                ecefToUsdTransform,
                gltfToEcefTransform,
                transform,
                gltf,
                primitive);
            // convertPrimitiveToFabric(
            //     stageInProgress,
            //     tilesetId,
            //     visible,
            //     primName,
            //     ecefToUsdTransform,
            //     gltfToEcefTransform,
            //     transform,
            //     gltf,
            //     primitive);
        });
}
} // namespace cesium::omniverse::GltfToUsd
