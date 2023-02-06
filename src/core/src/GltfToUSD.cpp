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
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
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
    (wrap_v));
}
// clang-format on

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

pxr::UsdShadeShader defineMdlShader_OmniPBR(
    pxr::UsdStageRefPtr stage,
    const pxr::SdfPath& materialPath,
    const pxr::TfToken& shaderName,
    const pxr::SdfAssetPath& assetPath,
    const pxr::TfToken& subIdentifier) {
    auto shader = pxr::UsdShadeShader::Define(stage, materialPath.AppendChild(shaderName));
    shader.SetSourceAsset(assetPath, pxr::_tokens->mdl);
    shader.SetSourceAssetSubIdentifier(subIdentifier, pxr::_tokens->mdl);
    shader.CreateIdAttr(pxr::VtValue(shaderName));

    return shader;
}

pxr::UsdShadeMaterial convertMaterialToUSD_OmniPBR(
    pxr::UsdStageRefPtr stage,
    usdrt::UsdStageRefPtr stageUsdrt,
    const std::string& materialName,
    const std::vector<pxr::SdfAssetPath>& textureUsdPaths,
    const bool hasRasterOverlay,
    const pxr::SdfAssetPath& rasterOverlayPath,
    const CesiumGltf::Material& material) {
    const pxr::SdfPath materialPath(fmt::format("/{}", materialName));
    const usdrt::SdfPath materialPathUsdrt(fmt::format("/{}", materialName));
    pxr::UsdShadeMaterial materialUsd = pxr::UsdShadeMaterial::Define(stage, materialPath);

    const auto& pbrMetallicRoughness = material.pbrMetallicRoughness;
    auto pbrShader = defineMdlShader_OmniPBR(
        stage, materialPath, pxr::_tokens->OmniPBR, pxr::SdfAssetPath("OmniPBR.mdl"), pxr::_tokens->OmniPBR);

    const auto setupDiffuseTexture = [&stage, &materialPath, &pbrShader](const pxr::SdfAssetPath& texturePath) {
        const auto nvidiaSupportDefinitions = pxr::SdfAssetPath("nvidia/support_definitions.mdl");

        // Set up texture_coordinates_2d shader.
        auto textureCoordinates2dShader = defineMdlShader_OmniPBR(
            stage,
            materialPath,
            pxr::_tokens->texture_coordinate_2d,
            nvidiaSupportDefinitions,
            pxr::_tokens->texture_coordinate_2d);

        const auto iTexCoordInput =
            textureCoordinates2dShader.CreateInput(pxr::_tokens->i, pxr::SdfValueTypeNames->Int);
        iTexCoordInput.Set(0);
        const auto textureCoordinates2dOutput =
            textureCoordinates2dShader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Float2);

        // Set up add & divide nodes for translation & scale.
        auto scalePrimvarReaderShader = defineMdlShader_OmniPBR(
            stage,
            materialPath,
            pxr::_tokens->scale_primvar_reader,
            nvidiaSupportDefinitions,
            pxr::_tokens->data_lookup_uniform_float2);

        scalePrimvarReaderShader.CreateInput(pxr::_tokens->default_value, pxr::SdfValueTypeNames->Float2)
            .Set(pxr::GfVec2f(1.0f, 1.0f));
        scalePrimvarReaderShader.CreateInput(pxr::_tokens->name, pxr::SdfValueTypeNames->String).Set(SCALE_PRIMVAR_ID);

        const auto scalePrimvarReaderOutput =
            scalePrimvarReaderShader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Float2);

        auto scaleShader = defineMdlShader_OmniPBR(
            stage,
            materialPath,
            pxr::_tokens->multiply,
            nvidiaSupportDefinitions,
            pxr::_tokens->multiply_subidentifier);

        const auto scaleAInput = scaleShader.CreateInput(pxr::_tokens->a, pxr::SdfValueTypeNames->Float2);
        scaleAInput.ConnectToSource(textureCoordinates2dOutput);
        const auto scaleBInput = scaleShader.CreateInput(pxr::_tokens->b, pxr::SdfValueTypeNames->Float2);
        scaleBInput.ConnectToSource(scalePrimvarReaderOutput);

        const auto scaleOutput = scaleShader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Float2);

        auto translationPrimvarReader = defineMdlShader_OmniPBR(
            stage,
            materialPath,
            pxr::_tokens->translation_primvar_reader,
            nvidiaSupportDefinitions,
            pxr::_tokens->data_lookup_uniform_float2);

        translationPrimvarReader.CreateInput(pxr::_tokens->default_value, pxr::SdfValueTypeNames->Float2)
            .Set(pxr::GfVec2f(0.0f, 0.0f));
        translationPrimvarReader.CreateInput(pxr::_tokens->name, pxr::SdfValueTypeNames->String)
            .Set(TRANSLATION_PRIMVAR_ID);

        const auto translatePrimvarReaderOutput =
            translationPrimvarReader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Float2);

        auto translationShader = defineMdlShader_OmniPBR(
            stage, materialPath, pxr::_tokens->add, nvidiaSupportDefinitions, pxr::_tokens->add_subidentifier);

        const auto translationAInput = translationShader.CreateInput(pxr::_tokens->a, pxr::SdfValueTypeNames->Float2);
        translationAInput.ConnectToSource(scaleOutput);
        const auto translationBInput = translationShader.CreateInput(pxr::_tokens->b, pxr::SdfValueTypeNames->Float2);
        translationBInput.ConnectToSource(translatePrimvarReaderOutput);

        const auto translationOutput =
            translationShader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Float2);

        // Set up lookup_color shader.
        auto lookupColorShader = defineMdlShader_OmniPBR(
            stage, materialPath, pxr::_tokens->lookup_color, nvidiaSupportDefinitions, pxr::_tokens->lookup_color);

        const auto lookupColorTexInput =
            lookupColorShader.CreateInput(pxr::_tokens->tex, pxr::SdfValueTypeNames->Asset);
        lookupColorTexInput.Set(texturePath);
        lookupColorShader.CreateOutput(pxr::_tokens->out, pxr::SdfValueTypeNames->Color3f);

        const auto lookupColorCoordInput =
            lookupColorShader.CreateInput(pxr::_tokens->coord, pxr::SdfValueTypeNames->Float2);
        lookupColorCoordInput.ConnectToSource(translationOutput);

        // Set uv wrapping to clamp. 0 = clamp. See
        // https://github.com/NVIDIA/MDL-SDK/blob/master/src/mdl/compiler/stdmodule/tex.mdl#L36
        lookupColorShader.CreateInput(pxr::_tokens->wrap_u, pxr::SdfValueTypeNames->Int).Set(0);
        lookupColorShader.CreateInput(pxr::_tokens->wrap_v, pxr::SdfValueTypeNames->Int).Set(0);

        const auto pbrShaderDiffuseColorInput =
            pbrShader.CreateInput(pxr::_tokens->diffuse_color_constant, pxr::SdfValueTypeNames->Color3f);
        pbrShaderDiffuseColorInput.ConnectToSource(lookupColorShader.GetOutput(pxr::_tokens->out));

        // TODO: Eventually we need to actually take the material values from Cesium Native and apply them.
        pbrShader.CreateInput(pxr::_tokens->reflection_roughness_constant, pxr::SdfValueTypeNames->Float).Set(0.1f);
    };

    if (hasRasterOverlay) {
        setupDiffuseTexture(rasterOverlayPath);
    } else if (pbrMetallicRoughness->baseColorTexture) {
        const auto baseColorIndex = static_cast<std::size_t>(pbrMetallicRoughness->baseColorTexture->index);
        const pxr::SdfAssetPath& texturePath = textureUsdPaths[baseColorIndex];
        setupDiffuseTexture(texturePath);
    } else {
        pbrShader.CreateInput(pxr::_tokens->diffuse_color_constant, pxr::SdfValueTypeNames->Vector3f)
            .Set(pxr::GfVec3f(
                static_cast<float>(pbrMetallicRoughness->baseColorFactor[0]),
                static_cast<float>(pbrMetallicRoughness->baseColorFactor[1]),
                static_cast<float>(pbrMetallicRoughness->baseColorFactor[2])));
    }

    materialUsd.CreateSurfaceOutput(pxr::_tokens->mdl).ConnectToSource(pbrShader.ConnectableAPI(), pxr::_tokens->out);
    materialUsd.CreateDisplacementOutput(pxr::_tokens->mdl)
        .ConnectToSource(pbrShader.ConnectableAPI(), pxr::_tokens->out);
    materialUsd.CreateVolumeOutput(pxr::_tokens->mdl).ConnectToSource(pbrShader.ConnectableAPI(), pxr::_tokens->out);

    // // Populate into Fabric
    // stageUsdrt->GetPrimAtPath(materialPathUsdrt);

    return materialUsd;
}

void convertPrimitiveToFabric(
    carb::flatcache::StageInProgress& stageInProgress,
    int tilesetId,
    int tileId,
    const std::string& primName,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const std::vector<pxr::UsdShadeMaterial>& materials) {

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

    const auto materialId = static_cast<uint64_t>(primitive.material);

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
    const carb::flatcache::Token vertexToken("vertex");
    const carb::flatcache::Token primvarsToken("primvars");
    const carb::flatcache::Token primvarInterpolationsToken("primvarInterpolations");
    const carb::flatcache::Token displayColorToken("primvars:displayColor");
    const carb::flatcache::Token stToken("primvars:st");
    const carb::flatcache::Token meshToken("Mesh");
    const carb::flatcache::Token tilesetIdToken("_tilesetId");
    const carb::flatcache::Token tileIdToken("_tileId");
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

    const carb::flatcache::Type tilesetIdType(
        carb::flatcache::BaseDataType::eInt, 1, 0, carb::flatcache::AttributeRole::eNone);

    const carb::flatcache::Type tileIdType(
        carb::flatcache::BaseDataType::eInt, 1, 0, carb::flatcache::AttributeRole::eNone);

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

    stageInProgress.createPrim(primPath);
    stageInProgress.createAttributes(
        primPath,
        std::array<carb::flatcache::AttrNameAndType, 17>{
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
                tilesetIdType,
                tilesetIdToken,
            },
            carb::flatcache::AttrNameAndType{
                tileIdType,
                tileIdToken,
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

    stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, faceVertexCounts.size());
    stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, indices.size());
    stageInProgress.setArrayAttributeSize(primPath, pointsToken, positions.size());
    stageInProgress.setArrayAttributeSize(primPath, primvarsToken, 2);
    stageInProgress.setArrayAttributeSize(primPath, primvarInterpolationsToken, 2);
    stageInProgress.setArrayAttributeSize(primPath, displayColorToken, 1);
    stageInProgress.setArrayAttributeSize(primPath, stToken, st0.size());

    auto faceVertexCountsFabric = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
    auto faceVertexIndicesFabric = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
    auto pointsFabric = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
    auto worldExtentFabric = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);
    auto visibilityFabric = stageInProgress.getAttributeWr<bool>(primPath, visibilityToken);
    auto primvarsFabric = stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarsToken);
    auto primvarInterpolationsFabric =
        stageInProgress.getArrayAttributeWr<carb::flatcache::TokenC>(primPath, primvarInterpolationsToken);
    auto displayColorFabric = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, displayColorToken);
    auto stFabric = stageInProgress.getArrayAttributeWr<usdrt::GfVec2f>(primPath, stToken);
    auto tilesetIdFabric = stageInProgress.getAttributeWr<int>(primPath, tilesetIdToken);
    auto tileIdFabric = stageInProgress.getAttributeWr<int>(primPath, tileIdToken);
    auto worldPositionFabric = stageInProgress.getAttributeWr<usdrt::GfVec3d>(primPath, worldPositionToken);
    auto worldOrientationFabric = stageInProgress.getAttributeWr<usdrt::GfQuatf>(primPath, worldOrientationToken);
    auto worldScaleFabric = stageInProgress.getAttributeWr<usdrt::GfVec3f>(primPath, worldScaleToken);
    auto localMatrixFabric = stageInProgress.getAttributeWr<usdrt::GfMatrix4d>(primPath, localMatrixToken);
    auto materialIdFabric = stageInProgress.getAttributeWr<carb::flatcache::TokenC>(primPath, materialIdToken);

    std::copy(faceVertexCounts.begin(), faceVertexCounts.end(), faceVertexCountsFabric.begin());
    std::copy(indices.begin(), indices.end(), faceVertexIndicesFabric.begin());
    std::copy(positions.begin(), positions.end(), pointsFabric.begin());
    std::copy(st0.begin(), st0.end(), stFabric.begin());

    worldExtentFabric->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
    worldExtentFabric->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));

    *visibilityFabric = false;
    primvarsFabric[0] = carb::flatcache::TokenC(displayColorToken);
    primvarsFabric[1] = carb::flatcache::TokenC(stToken);
    primvarInterpolationsFabric[0] = carb::flatcache::TokenC(constantToken);
    primvarInterpolationsFabric[1] = carb::flatcache::TokenC(vertexToken);
    displayColorFabric[0] = displayColor;
    *tilesetIdFabric = tilesetId;
    *tileIdFabric = tileId;
    *worldPositionFabric = worldPosition;
    *worldOrientationFabric = worldOrientation;
    *worldScaleFabric = worldScale;
    *localMatrixFabric = UsdUtil::glmToUsdrtMatrix(localToEcefTransform);

    if (materialId >= 0) {
        const auto& materialUsd = materials[materialId];
        *materialIdFabric = carb::flatcache::TokenC(carb::flatcache::Token(materialUsd.GetPath().GetText()));
    }
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
    int tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& tileTransform,
    const CesiumGltf::Model& model) {
    const auto stageUsd = UsdUtil::getUsdStage(stageId);
    const auto stageUsdrt = UsdUtil::getUsdrtStage(stageId);
    auto stageInProgressFabric = UsdUtil::getFabricStageInProgress(stageId);

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

    std::vector<pxr::UsdShadeMaterial> materialUSDs;
    materialUSDs.reserve(model.materials.size());

    for (auto i = 0; i < model.materials.size(); i++) {
        const auto materialName = fmt::format("{}_material_{}", tileName, i);

        materialUSDs.emplace_back(convertMaterialToUSD_OmniPBR(
            stageUsd, stageUsdrt, materialName, textureUsdPaths, false, pxr::SdfAssetPath(), model.materials[i]));
    }

    uint64_t primitiveId = 0;

    model.forEachPrimitiveInScene(
        -1,
        [&stageUsdrt,
         &stageInProgressFabric,
         &tileName,
         tilesetId,
         tileId,
         &primitiveId,
         &ecefToUsdTransform,
         &gltfToEcefTransform,
         &materialUSDs](
            const CesiumGltf::Model& gltf,
            [[maybe_unused]] const CesiumGltf::Node& node,
            [[maybe_unused]] const CesiumGltf::Mesh& mesh,
            const CesiumGltf::MeshPrimitive& primitive,
            const glm::dmat4& transform) {
            const auto primName = fmt::format("{}_primitive_{}", tileName, primitiveId++);
            // convertPrimitiveToUsdrt(
            //     stage,
            //     tilesetId,
            //     primName,
            //     ecefToUsdTransform,
            //     gltfToEcefTransform,
            //     transform,
            //     gltf,
            //     primitive);
            convertPrimitiveToFabric(
                stageInProgressFabric,
                tilesetId,
                tileId,
                primName,
                ecefToUsdTransform,
                gltfToEcefTransform,
                transform,
                gltf,
                primitive,
                materialUSDs);
        });
}
} // namespace cesium::omniverse::GltfToUsd
