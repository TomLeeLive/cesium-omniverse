#include "cesium/omniverse/GltfToUSD.h"

#include "cesium/omniverse/UsdUtil.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGeometry/AxisTransforms.h>
#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/Model.h>
#include <CesiumGltfReader/GltfReader.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/fmt/fmt.h>
#include <usdrt/gf/range.h>
#include <usdrt/gf/vec.h>
#include <usdrt/scenegraph/usd/rt/xformable.h>
#include <usdrt/scenegraph/usd/sdf/types.h>

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

std::mutex mutex;

void convertPrimitiveToUsd(
    const usdrt::UsdStageRefPtr& stage,
    int tilesetId,
    const std::string& nodeName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    uint64_t primitiveIndex) {

    std::scoped_lock<std::mutex> lock(mutex);

    usdrt::VtArray<usdrt::GfVec3f> positions = getPrimitivePositions(model, primitive);
    usdrt::VtArray<int> indices = getPrimitiveIndices(model, primitive, positions);
    usdrt::VtArray<usdrt::GfVec3f> normals = getPrimitiveNormals(model, primitive, positions, indices);
    usdrt::VtArray<usdrt::GfVec2f> st0 = getPrimitiveUVs(model, primitive, 0);
    usdrt::VtArray<usdrt::GfVec2f> st1 = getPrimitiveUVs(model, primitive, 1);

    // TODO: does world extent actually have to be the world extent after transformations have been applied?
    std::optional<usdrt::GfRange3d> worldExtent = getPrimitiveExtent(model, primitive);

    if (positions.empty() || indices.empty() || normals.empty() || !worldExtent.has_value()) {
        return;
    }

    const auto faceCount = indices.size() / 3;
    usdrt::VtArray<int> faceVertexCounts;
    faceVertexCounts.reserve(faceCount);

    for (auto i = 0; i < faceCount; i++) {
        faceVertexCounts.push_back(3);
    }

    usdrt::VtArray<usdrt::GfVec3f> displayColor = {usdrt::GfVec3f(1.0, 0.0, 0.0)};

    std::string primName = fmt::format("{}_mesh_primitive_{}", nodeName, primitiveIndex);

    auto localToEcefTransform = gltfToEcefTransform * nodeTransform;
    auto localToUsdTransform = ecefToUsdTransform * localToEcefTransform;
    auto [worldPosition, worldOrientation, worldScale] = UsdUtil::glmToUsdrtMatrixDecomposed(localToUsdTransform);

    (void)worldPosition;
    (void)worldOrientation;
    (void)worldScale;

    // TODO: precreate tokens
    usdrt::TfToken meshToken("Mesh");
    usdrt::TfToken faceVertexCountsToken("faceVertexCounts");
    usdrt::TfToken faceVertexIndicesToken("faceVertexIndices");
    usdrt::TfToken pointsToken("points");
    usdrt::TfToken displayColorToken("primvars:displayColor");
    usdrt::TfToken worldExtentToken("_worldExtent");
    usdrt::TfToken constantToken("constant");
    usdrt::TfToken primvarsToken("primvars");
    usdrt::TfToken primvarInterpolationsToken("primvarInterpolations");
    usdrt::TfToken tilesetIdToken("_tilesetId");

    const usdrt::SdfPath primPath(fmt::format("/{}", primName));
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
    prim.CreateAttribute(displayColorToken, usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
    prim.CreateAttribute(worldExtentToken, usdrt::SdfValueTypeNames->Range3d, false).Set(worldExtent.value());
    prim.CreateAttribute(tilesetIdToken, usdrt::SdfValueTypeNames->Int, true).Set(tilesetId);

    // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
    // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
    // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
    const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(displayColorToken)};
    const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(constantToken)};

    prim.CreateAttribute(primvarsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
    prim.CreateAttribute(primvarInterpolationsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvarInterp);
}

void convertMeshToUsd(
    const usdrt::UsdStageRefPtr& stage,
    int tilesetId,
    const std::string& nodeName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::Mesh& mesh) {
    for (auto i = 0; i < mesh.primitives.size(); i++) {
        const auto& primitive = mesh.primitives[i];
        convertPrimitiveToUsd(
            stage, tilesetId, nodeName, ecefToUsdTransform, gltfToEcefTransform, nodeTransform, model, primitive, i);
    }
}

void convertNodeToUsdRecursive(
    const usdrt::UsdStageRefPtr& stage,
    int tilesetId,
    const std::string& parentName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& parentTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::Node& node,
    uint64_t nodeIndex) {
    const auto nodeName = fmt::format("{}_node_{}", parentName, nodeIndex);
    const auto nodeTransform = parentTransform * getNodeMatrix(node);
    for (auto child : node.children) {
        const auto childIndex = static_cast<uint64_t>(child);
        if (childIndex >= 0 && childIndex < model.nodes.size()) {
            convertNodeToUsdRecursive(
                stage,
                tilesetId,
                nodeName,
                ecefToUsdTransform,
                gltfToEcefTransform,
                nodeTransform,
                model,
                model.nodes[childIndex],
                childIndex);
        }
    }

    auto meshIndex = static_cast<uint64_t>(node.mesh);
    if (meshIndex < model.meshes.size()) {
        convertMeshToUsd(
            stage,
            tilesetId,
            nodeName,
            ecefToUsdTransform,
            gltfToEcefTransform,
            nodeTransform,
            model,
            model.meshes[meshIndex]);
    }
}

} // namespace

void createUsdrtPrims(
    long stageId,
    int tilesetId,
    uint64_t tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& tileTransform,
    const std::string& tilesetName,
    const CesiumGltf::Model& model) {
    const auto stage = UsdUtil::getUsdrtStage(stageId);

    const auto gltfToEcefTransform = tileTransform * CesiumGeometry::AxisTransforms::Y_UP_TO_Z_UP;
    const auto parentTransform = glm::dmat4(1.0);
    const auto parentName = fmt::format("{}_tile_{}", tilesetName, tileId);

    const auto sceneIndex = static_cast<uint64_t>(model.scene);
    if (sceneIndex >= 0 && sceneIndex < model.scenes.size()) {
        const auto& scene = model.scenes[sceneIndex];
        for (const auto node : scene.nodes) {
            const auto nodeIndex = static_cast<uint64_t>(node);
            if (nodeIndex >= 0 && nodeIndex < model.nodes.size()) {
                convertNodeToUsdRecursive(
                    stage,
                    tilesetId,
                    parentName,
                    ecefToUsdTransform,
                    gltfToEcefTransform,
                    parentTransform,
                    model,
                    model.nodes[nodeIndex],
                    nodeIndex);
            }
        }
    } else if (!model.nodes.empty()) {
        for (auto nodeIndex = 0; nodeIndex < model.nodes.size(); nodeIndex++) {
            convertNodeToUsdRecursive(
                stage,
                tilesetId,
                parentName,
                ecefToUsdTransform,
                gltfToEcefTransform,
                parentTransform,
                model,
                model.nodes[nodeIndex],
                nodeIndex);
        }
    } else {
        for (const auto& mesh : model.meshes) {
            convertMeshToUsd(
                stage, tilesetId, parentName, ecefToUsdTransform, gltfToEcefTransform, parentTransform, model, mesh);
        }
    }
}
} // namespace cesium::omniverse::GltfToUsd
