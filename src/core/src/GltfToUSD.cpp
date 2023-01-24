#include "cesium/omniverse/GltfToUSD.h"

#include "cesium/omniverse/UsdUtil.h"

#ifdef CESIUM_OMNI_MSVC
#pragma push_macro("OPAQUE")
#undef OPAQUE
#endif

#include <CesiumGltf/AccessorView.h>
#include <CesiumGltf/Model.h>
#include <CesiumGltfReader/GltfReader.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <spdlog/fmt/fmt.h>
#include <usdrt/gf/range.h>
#include <usdrt/gf/vec.h>
#include <usdrt/scenegraph/usd/sdf/types.h>

#include <numeric>
#include <optional>

namespace cesium::omniverse {

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
    usdPositions.reserve(static_cast<std::size_t>(positions.size()));
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
        indices.reserve(static_cast<std::size_t>(indicesAccessorView.size()));
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
        indices.reserve(static_cast<std::size_t>(indicesAccessorView.size() - 2) * 3);
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
        indices.reserve(static_cast<std::size_t>(indicesAccessorView.size() - 2) * 3);
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
        indices.resize(positions.size());
        std::iota(indices.begin(), indices.end(), 0);
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
            normalsUsd.reserve(static_cast<std::size_t>(normalsView.size()));
            for (auto i = 0; i < normalsView.size(); ++i) {
                const glm::vec3& n = normalsView[i];
                normalsUsd.push_back(usdrt::GfVec3f(n.x, n.y, n.z));
            }

            return normalsUsd;
        }
    }

    // Generate normals
    // TODO: confirm that VtArray elements are initialized to 0
    usdrt::VtArray<usdrt::GfVec3f> normalsUsd(positions.size());
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
    usdUVs.reserve(static_cast<size_t>(uvs.size()));
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

void convertPrimitiveToUsd(
    usdrt::UsdStageRefPtr& stage,
    const std::string& parentName,
    const glm::dmat4& parentMatrix,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const uint64_t primitiveIndex) {

    // TODO
    (void)parentMatrix;

    usdrt::VtArray<usdrt::GfVec3f> positions = getPrimitivePositions(model, primitive);
    usdrt::VtArray<int> indices = getPrimitiveIndices(model, primitive, positions);
    usdrt::VtArray<usdrt::GfVec3f> normals = getPrimitiveNormals(model, primitive, positions, indices);
    usdrt::VtArray<usdrt::GfVec2f> st0 = getPrimitiveUVs(model, primitive, 0);
    usdrt::VtArray<usdrt::GfVec2f> st1 = getPrimitiveUVs(model, primitive, 1);

    // TODO: check if it's ok to set this to the local extent ()
    std::optional<usdrt::GfRange3d> worldExtent = getPrimitiveExtent(model, primitive);

    if (positions.empty() || indices.empty() || normals.empty() || !worldExtent.has_value()) {
        return;
    }

    usdrt::VtArray<int> faceVertexCounts(indices.size() / 3);
    std::fill(faceVertexCounts.begin(), faceVertexCounts.end(), 3);

    usdrt::VtArray<usdrt::GfVec3f> displayColor = {usdrt::GfVec3f(1.0, 0.0, 0.0)};

    std::string computedName = fmt::format("{}_mesh_primitive_{}", parentName, primitiveIndex);

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

    const usdrt::SdfPath primPath(fmt::format("/{}", computedName));
    const usdrt::UsdPrim prim = stage->DefinePrim(primPath, meshToken);

    // TODO: normals
    prim.CreateAttribute(faceVertexCountsToken, usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexCounts);
    prim.CreateAttribute(faceVertexIndicesToken, usdrt::SdfValueTypeNames->IntArray, false).Set(indices);
    prim.CreateAttribute(pointsToken, usdrt::SdfValueTypeNames->Point3fArray, false).Set(positions);
    prim.CreateAttribute(displayColorToken, usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
    prim.CreateAttribute(worldExtentToken, usdrt::SdfValueTypeNames->Range3d, false).Set(worldExtent.value());

    // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
    // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
    // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
    const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(displayColorToken)};
    const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(constantToken)};

    prim.CreateAttribute(primvarsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
    prim.CreateAttribute(primvarInterpolationsToken, usdrt::SdfValueTypeNames->TokenArray, false).Set(primvarInterp);
}

void convertMeshToUsd(
    usdrt::UsdStageRefPtr& stage,
    const std::string& parentName,
    const glm::dmat4& parentMatrix,
    const CesiumGltf::Model& model,
    const CesiumGltf::Mesh& mesh) {
    for (auto i = 0; i < mesh.primitives.size(); i++) {
        const auto& primitive = mesh.primitives[i];
        convertPrimitiveToUsd(stage, parentName, parentMatrix, model, primitive, i);
    }
}

void convertNodeToUsd(
    usdrt::UsdStageRefPtr& stage,
    const std::string& parentName,
    const glm::dmat4& parentMatrix,
    const CesiumGltf::Model& model,
    const CesiumGltf::Node& node,
    uint64_t nodeIndex) {
    std::string computedName = fmt::format("{}_node_{}", parentName, nodeIndex);
    glm::dmat4 computedMatrix = parentMatrix * getNodeMatrix(node);
    for (int32_t child : node.children) {
        if (child >= 0 && child < static_cast<int32_t>(model.nodes.size())) {
            convertNodeToUsd(
                stage,
                computedName,
                computedMatrix,
                model,
                model.nodes[static_cast<size_t>(child)],
                static_cast<uint64_t>(child));
        }
    }

    auto meshIndex = static_cast<uint64_t>(node.mesh);
    if (meshIndex < static_cast<int32_t>(model.meshes.size())) {
        convertMeshToUsd(stage, computedName, computedMatrix, model, model.meshes[meshIndex]);
    }
}

} // namespace

void createUsdrtPrim(long stageId, const std::string& tilesetName, const CesiumGltf::Model& model) {
    auto stage = getUsdrtStage(stageId);

    const auto sceneIdx = model.scene;
    if (sceneIdx >= 0 && sceneIdx < static_cast<int32_t>(model.scenes.size())) {
        const auto& scene = model.scenes[static_cast<size_t>(sceneIdx)];
        for (const auto node : scene.nodes) {
            if (node >= 0 && node < static_cast<int32_t>(model.nodes.size())) {
                convertNodeToUsd(
                    stage,
                    tilesetName,
                    glm::dmat4(1.0),
                    model,
                    model.nodes[static_cast<size_t>(node)],
                    static_cast<uint64_t>(node));
            }
        }
    } else if (!model.nodes.empty()) {
        for (auto i = 0; i < model.nodes.size(); i++) {
            convertNodeToUsd(stage, tilesetName, glm::dmat4(1.0), model, model.nodes[i], i);
        }
    } else {
        for (const auto& mesh : model.meshes) {
            convertMeshToUsd(stage, tilesetName, glm::dmat4(1.0), model, mesh);
        }
    }
}
} // namespace cesium::omniverse
