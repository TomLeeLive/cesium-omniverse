#include "cesium/omniverse/FabricMaterialPool.h"

#include "cesium/omniverse/DynamicTextureProviderCache.h"
#include "cesium/omniverse/FabricAsset.h"
#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/LoggerSink.h"
#include "cesium/omniverse/Tokens.h"
#include "cesium/omniverse/UsdUtil.h"

#include <carb/flatcache/FlatCacheUSD.h>
#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse {

FabricMaterialPool::FabricMaterialPool() {}

void FabricMaterialPool::initialize() {
    if (_items.size() > 0) {
        // Already initialized
        return;
    }

    const size_t initialCapacity = 1000;

    for (size_t i = 0; i < initialCapacity; i++) {
        const auto materialPath = pxr::SdfPath(fmt::format("/fabric_material_pool_item_{}", i));
        const auto textureName = fmt::format("fabric_texture_pool_item_{}", i);
        _items.emplace_back(FabricMaterialPoolItem(materialPath, textureName));
    }
}

pxr::SdfPath FabricMaterialPool::get() {
    for (auto& item : _items) {
        if (!item._occupied) {
            item._occupied = true;
            return item._path;
        }
    }

    CESIUM_LOG_ERROR("Nothing left in pool");
    return {};
}

void FabricMaterialPool::release(const pxr::SdfPath& path) {
    for (auto& item : _items) {
        if (item._occupied && item._path == path) {
            item._occupied = false;
            return;
        }
    }

    CESIUM_LOG_ERROR("Path not found in pool");
}

struct AssetPath {
    std::string assetName;
    pxr::SdfAssetPath assetPath;
};

AssetPath getAssetPath(const std::string& name) {
    const auto texturePath = fmt::format("{}{}", rtx::resourcemanager::kDynamicTexturePrefix, name);
    const auto assetPath = pxr::SdfAssetPath(texturePath);

    return AssetPath{name, assetPath};
}

pxr::SdfAssetPath addTexture(const std::string& assetName) {
    auto provider = std::make_unique<omni::ui::DynamicTextureProvider>(assetName);

    std::vector<uint8_t> pixelData = {255, 255, 0, 255};

    provider->setBytesData(
        pixelData.data(), carb::Uint2{1, 1}, omni::ui::kAutoCalculateStride, carb::Format::eRGBA8_SRGB);

    auto& cache = DynamicTextureProviderCache::getInstance();
    cache.insert(assetName, std::move(provider));

    return getAssetPath(assetName).assetPath;
}

FabricMaterialPoolItem::FabricMaterialPoolItem(const pxr::SdfPath& materialPath, const std::string& textureName)
    : _path(materialPath)
    , _occupied(false) {

    auto textureAssetPath = addTexture(textureName);

    auto sip = UsdUtil::getFabricStageInProgress();

    const auto uvTranslation = glm::dvec2(0.0, 0.0);
    const auto uvScale = glm::dvec2(1.0, 1.0);

    const auto metallicFactor = 0.0f;
    const auto roughnessFactor = 1.0f;
    const auto specularFactor = 0.0f;

    const auto shaderPath = materialPath.AppendChild(UsdTokens::Shader);
    const auto displacementPath = materialPath.AppendChild(UsdTokens::displacement);
    const auto surfacePath = materialPath.AppendChild(UsdTokens::surface);
    const auto multiplyPath = materialPath.AppendChild(UsdTokens::multiply);
    const auto addPath = materialPath.AppendChild(UsdTokens::add);
    const auto lookupColorPath = materialPath.AppendChild(UsdTokens::lookup_color);
    const auto textureCoordinate2dPath = materialPath.AppendChild(UsdTokens::texture_coordinate_2d);

    const auto shaderPathFabricUint64 = carb::flatcache::asInt(shaderPath).path;
    const auto multiplyPathFabricUint64 = carb::flatcache::asInt(multiplyPath).path;
    const auto addPathFabricUint64 = carb::flatcache::asInt(addPath).path;
    const auto lookupColorPathFabricUint64 = carb::flatcache::asInt(lookupColorPath).path;
    const auto textureCoordinate2dPathFabricUint64 = carb::flatcache::asInt(textureCoordinate2dPath).path;

    // Material
    {
        const auto materialPathFabric = carb::flatcache::Path(carb::flatcache::asInt(materialPath));
        sip.createPrim(materialPathFabric);

        FabricAttributesBuilder attributes;

        attributes.addAttribute(FabricTypes::_terminals, FabricTokens::_terminals);
        attributes.addAttribute(FabricTypes::Material, FabricTokens::Material);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);

        attributes.createAttributes(materialPathFabric);

        sip.setArrayAttributeSize(materialPathFabric, FabricTokens::_terminals, 2);

        auto terminalsFabric = sip.getArrayAttributeWr<uint64_t>(materialPathFabric, FabricTokens::_terminals);

        terminalsFabric[0] = shaderPathFabricUint64;
        terminalsFabric[1] = shaderPathFabricUint64;
    }

    // Displacement
    {
        const auto displacementPathFabric = carb::flatcache::Path(carb::flatcache::asInt(displacementPath));
        sip.createPrim(displacementPathFabric);

        FabricAttributesBuilder attributes;

        attributes.addAttribute(FabricTypes::_nodePaths, FabricTokens::_nodePaths);
        attributes.addAttribute(FabricTypes::_relationships_inputId, FabricTokens::_relationships_inputId);
        attributes.addAttribute(FabricTypes::_relationships_outputId, FabricTokens::_relationships_outputId);
        attributes.addAttribute(FabricTypes::_relationships_inputName, FabricTokens::_relationships_inputName);
        attributes.addAttribute(FabricTypes::_relationships_outputName, FabricTokens::_relationships_outputName);
        attributes.addAttribute(FabricTypes::primvars, FabricTokens::primvars);
        attributes.addAttribute(FabricTypes::MaterialNetwork, FabricTokens::MaterialNetwork);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);

        attributes.createAttributes(displacementPathFabric);

        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::_nodePaths, 5);
        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::_relationships_inputId, 4);
        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::_relationships_outputId, 4);
        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::_relationships_inputName, 4);
        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::_relationships_outputName, 4);
        sip.setArrayAttributeSize(displacementPathFabric, FabricTokens::primvars, 0);

        // clang-format off
        auto nodePathsFabric = sip.getArrayAttributeWr<uint64_t>(displacementPathFabric, FabricTokens::_nodePaths);
        auto relationshipsInputIdFabric = sip.getArrayAttributeWr<uint64_t>(displacementPathFabric, FabricTokens::_relationships_inputId);
        auto relationshipsOutputIdFabric = sip.getArrayAttributeWr<uint64_t>(displacementPathFabric, FabricTokens::_relationships_outputId);
        auto relationshipsInputNameFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(displacementPathFabric, FabricTokens::_relationships_inputName);
        auto relationshipsOutputNameFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(displacementPathFabric, FabricTokens::_relationships_outputName);
        // clang-format on

        nodePathsFabric[0] = textureCoordinate2dPathFabricUint64;
        nodePathsFabric[1] = multiplyPathFabricUint64;
        nodePathsFabric[2] = addPathFabricUint64;
        nodePathsFabric[3] = lookupColorPathFabricUint64;
        nodePathsFabric[4] = shaderPathFabricUint64;

        relationshipsInputIdFabric[0] = textureCoordinate2dPathFabricUint64;
        relationshipsInputIdFabric[1] = multiplyPathFabricUint64;
        relationshipsInputIdFabric[2] = addPathFabricUint64;
        relationshipsInputIdFabric[3] = lookupColorPathFabricUint64;

        relationshipsOutputIdFabric[0] = multiplyPathFabricUint64;
        relationshipsOutputIdFabric[1] = addPathFabricUint64;
        relationshipsOutputIdFabric[2] = lookupColorPathFabricUint64;
        relationshipsOutputIdFabric[3] = lookupColorPathFabricUint64;

        relationshipsInputNameFabric[0] = FabricTokens::out;
        relationshipsInputNameFabric[1] = FabricTokens::out;
        relationshipsInputNameFabric[2] = FabricTokens::out;
        relationshipsInputNameFabric[3] = FabricTokens::out;

        relationshipsOutputNameFabric[0] = FabricTokens::a;
        relationshipsOutputNameFabric[1] = FabricTokens::a;
        relationshipsOutputNameFabric[2] = FabricTokens::coord;
        relationshipsOutputNameFabric[3] = FabricTokens::diffuse_color_constant;
    }

    // Surface
    {
        const auto surfacePathFabric = carb::flatcache::Path(carb::flatcache::asInt(surfacePath));
        sip.createPrim(surfacePathFabric);

        FabricAttributesBuilder attributes;

        attributes.addAttribute(FabricTypes::_nodePaths, FabricTokens::_nodePaths);
        attributes.addAttribute(FabricTypes::_relationships_inputId, FabricTokens::_relationships_inputId);
        attributes.addAttribute(FabricTypes::_relationships_outputId, FabricTokens::_relationships_outputId);
        attributes.addAttribute(FabricTypes::_relationships_inputName, FabricTokens::_relationships_inputName);
        attributes.addAttribute(FabricTypes::_relationships_outputName, FabricTokens::_relationships_outputName);
        attributes.addAttribute(FabricTypes::primvars, FabricTokens::primvars);
        attributes.addAttribute(FabricTypes::MaterialNetwork, FabricTokens::MaterialNetwork);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);

        attributes.createAttributes(surfacePathFabric);

        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::_nodePaths, 5);
        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::_relationships_inputId, 4);
        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::_relationships_outputId, 4);
        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::_relationships_inputName, 4);
        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::_relationships_outputName, 4);
        sip.setArrayAttributeSize(surfacePathFabric, FabricTokens::primvars, 0);

        // clang-format off
        auto nodePathsFabric = sip.getArrayAttributeWr<uint64_t>(surfacePathFabric, FabricTokens::_nodePaths);
        auto relationshipsInputIdFabric = sip.getArrayAttributeWr<uint64_t>(surfacePathFabric, FabricTokens::_relationships_inputId);
        auto relationshipsOutputIdFabric = sip.getArrayAttributeWr<uint64_t>(surfacePathFabric, FabricTokens::_relationships_outputId);
        auto relationshipsInputNameFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(surfacePathFabric, FabricTokens::_relationships_inputName);
        auto relationshipsOutputNameFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(surfacePathFabric, FabricTokens::_relationships_outputName);
        // clang-format on

        nodePathsFabric[0] = textureCoordinate2dPathFabricUint64;
        nodePathsFabric[1] = multiplyPathFabricUint64;
        nodePathsFabric[2] = addPathFabricUint64;
        nodePathsFabric[3] = lookupColorPathFabricUint64;
        nodePathsFabric[4] = shaderPathFabricUint64;

        relationshipsInputIdFabric[0] = textureCoordinate2dPathFabricUint64;
        relationshipsInputIdFabric[1] = multiplyPathFabricUint64;
        relationshipsInputIdFabric[2] = addPathFabricUint64;
        relationshipsInputIdFabric[3] = lookupColorPathFabricUint64;

        relationshipsOutputIdFabric[0] = multiplyPathFabricUint64;
        relationshipsOutputIdFabric[1] = addPathFabricUint64;
        relationshipsOutputIdFabric[2] = lookupColorPathFabricUint64;
        relationshipsOutputIdFabric[3] = shaderPathFabricUint64;

        relationshipsInputNameFabric[0] = FabricTokens::out;
        relationshipsInputNameFabric[1] = FabricTokens::out;
        relationshipsInputNameFabric[2] = FabricTokens::out;
        relationshipsInputNameFabric[3] = FabricTokens::out;

        relationshipsOutputNameFabric[0] = FabricTokens::a;
        relationshipsOutputNameFabric[1] = FabricTokens::a;
        relationshipsOutputNameFabric[2] = FabricTokens::coord;
        relationshipsOutputNameFabric[3] = FabricTokens::diffuse_color_constant;
    }

    // Shader
    {
        const auto shaderPathFabric = carb::flatcache::Path(carb::flatcache::asInt(shaderPath));
        sip.createPrim(shaderPathFabric);

        FabricAttributesBuilder attributes;

        // clang-format off
        attributes.addAttribute(FabricTypes::albedo_add, FabricTokens::albedo_add);
        attributes.addAttribute(FabricTypes::metallic_constant, FabricTokens::metallic_constant);
        attributes.addAttribute(FabricTypes::reflection_roughness_constant, FabricTokens::reflection_roughness_constant);
        attributes.addAttribute(FabricTypes::specular_level, FabricTokens::specular_level);
        attributes.addAttribute(FabricTypes::info_id, FabricTokens::info_id);
        attributes.addAttribute(FabricTypes::info_sourceAsset_subIdentifier, FabricTokens::info_sourceAsset_subIdentifier);
        attributes.addAttribute(FabricTypes::_paramColorSpace, FabricTokens::_paramColorSpace);
        attributes.addAttribute(FabricTypes::_parameters, FabricTokens::_parameters);
        attributes.addAttribute(FabricTypes::Shader, FabricTokens::Shader);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
        // clang-format on

        attributes.createAttributes(shaderPathFabric);

        sip.setArrayAttributeSize(shaderPathFabric, FabricTokens::_paramColorSpace, 0);
        sip.setArrayAttributeSize(shaderPathFabric, FabricTokens::_parameters, 4);

        // clang-format off
        auto albedoAddFabric = sip.getAttributeWr<float>(shaderPathFabric, FabricTokens::albedo_add);
        auto metallicConstantFabric = sip.getAttributeWr<float>(shaderPathFabric, FabricTokens::metallic_constant);
        auto reflectionRoughnessConstantFabric = sip.getAttributeWr<float>(shaderPathFabric, FabricTokens::reflection_roughness_constant);
        auto specularLevelFabric = sip.getAttributeWr<float>(shaderPathFabric, FabricTokens::specular_level);
        auto infoIdFabric = sip.getAttributeWr<carb::flatcache::Token>(shaderPathFabric, FabricTokens::info_id);
        auto infoSourceAssetSubIdentifierFabric = sip.getAttributeWr<carb::flatcache::Token>(shaderPathFabric, FabricTokens::info_sourceAsset_subIdentifier);
        auto parametersFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(shaderPathFabric, FabricTokens::_parameters);
        // clang-format on

        *albedoAddFabric = 0.0f;
        *metallicConstantFabric = metallicFactor;
        *reflectionRoughnessConstantFabric = roughnessFactor;
        *specularLevelFabric = specularFactor;
        *infoIdFabric = FabricTokens::OmniPBR_mdl;
        *infoSourceAssetSubIdentifierFabric = FabricTokens::OmniPBR;
        parametersFabric[0] = FabricTokens::albedo_add;
        parametersFabric[1] = FabricTokens::metallic_constant;
        parametersFabric[2] = FabricTokens::reflection_roughness_constant;
        parametersFabric[3] = FabricTokens::specular_level;
    }

    // texture_coordinate_2d
    {
        const auto textureCoordinate2dPathFabric =
            carb::flatcache::Path(carb::flatcache::asInt(textureCoordinate2dPath));
        sip.createPrim(textureCoordinate2dPathFabric);

        FabricAttributesBuilder attributes;

        // clang-format off
        attributes.addAttribute(FabricTypes::info_id, FabricTokens::info_id);
        attributes.addAttribute(FabricTypes::info_sourceAsset_subIdentifier, FabricTokens::info_sourceAsset_subIdentifier);
        attributes.addAttribute(FabricTypes::_paramColorSpace, FabricTokens::_paramColorSpace);
        attributes.addAttribute(FabricTypes::_parameters, FabricTokens::_parameters);
        attributes.addAttribute(FabricTypes::Shader, FabricTokens::Shader);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
        // clang-format on

        attributes.createAttributes(textureCoordinate2dPathFabric);

        sip.setArrayAttributeSize(textureCoordinate2dPathFabric, FabricTokens::_paramColorSpace, 0);
        sip.setArrayAttributeSize(textureCoordinate2dPathFabric, FabricTokens::_parameters, 0);

        // clang-format off
        auto infoIdFabric = sip.getAttributeWr<carb::flatcache::Token>(textureCoordinate2dPathFabric, FabricTokens::info_id);
        auto infoSourceAssetSubIdentifierFabric = sip.getAttributeWr<carb::flatcache::Token>(textureCoordinate2dPathFabric, FabricTokens::info_sourceAsset_subIdentifier);
        // clang-format on

        *infoIdFabric = FabricTokens::nvidia_support_definitions_mdl;
        *infoSourceAssetSubIdentifierFabric = FabricTokens::texture_coordinate_2d;
    }

    // multiply
    {
        const auto multiplyPathFabric = carb::flatcache::Path(carb::flatcache::asInt(multiplyPath));
        sip.createPrim(multiplyPathFabric);

        FabricAttributesBuilder attributes;

        // clang-format off
        attributes.addAttribute(FabricTypes::b, FabricTokens::b);
        attributes.addAttribute(FabricTypes::info_id, FabricTokens::info_id);
        attributes.addAttribute(FabricTypes::info_sourceAsset_subIdentifier, FabricTokens::info_sourceAsset_subIdentifier);
        attributes.addAttribute(FabricTypes::_paramColorSpace, FabricTokens::_paramColorSpace);
        attributes.addAttribute(FabricTypes::_parameters, FabricTokens::_parameters);
        attributes.addAttribute(FabricTypes::Shader, FabricTokens::Shader);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
        // clang-format on

        attributes.createAttributes(multiplyPathFabric);

        sip.setArrayAttributeSize(multiplyPathFabric, FabricTokens::_paramColorSpace, 0);
        sip.setArrayAttributeSize(multiplyPathFabric, FabricTokens::_parameters, 1);

        // clang-format off
        auto bFabric = sip.getAttributeWr<pxr::GfVec2f>(multiplyPathFabric, FabricTokens::b);
        auto infoIdFabric = sip.getAttributeWr<carb::flatcache::Token>(multiplyPathFabric, FabricTokens::info_id);
        auto infoSourceAssetSubIdentifierFabric = sip.getAttributeWr<carb::flatcache::Token>(multiplyPathFabric, FabricTokens::info_sourceAsset_subIdentifier);
        auto parametersFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(multiplyPathFabric, FabricTokens::_parameters);
        // clang-format on

        *bFabric = pxr::GfVec2f(static_cast<float>(uvScale.x), static_cast<float>(uvScale.y));
        *infoIdFabric = FabricTokens::nvidia_support_definitions_mdl;
        *infoSourceAssetSubIdentifierFabric = FabricTokens::multiply_float2_float2;
        parametersFabric[0] = FabricTokens::b;
    }

    // add
    {
        const auto addPathFabric = carb::flatcache::Path(carb::flatcache::asInt(addPath));
        sip.createPrim(addPathFabric);

        FabricAttributesBuilder attributes;

        // clang-format off
        attributes.addAttribute(FabricTypes::b, FabricTokens::b);
        attributes.addAttribute(FabricTypes::info_id, FabricTokens::info_id);
        attributes.addAttribute(FabricTypes::info_sourceAsset_subIdentifier, FabricTokens::info_sourceAsset_subIdentifier);
        attributes.addAttribute(FabricTypes::_paramColorSpace, FabricTokens::_paramColorSpace);
        attributes.addAttribute(FabricTypes::_parameters, FabricTokens::_parameters);
        attributes.addAttribute(FabricTypes::Shader, FabricTokens::Shader);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
        // clang-format on

        attributes.createAttributes(addPathFabric);

        sip.setArrayAttributeSize(addPathFabric, FabricTokens::_paramColorSpace, 0);
        sip.setArrayAttributeSize(addPathFabric, FabricTokens::_parameters, 1);

        // clang-format off
        auto bFabric = sip.getAttributeWr<pxr::GfVec2f>(addPathFabric, FabricTokens::b);
        auto infoIdFabric = sip.getAttributeWr<carb::flatcache::Token>(addPathFabric, FabricTokens::info_id);
        auto infoSourceAssetSubIdentifierFabric = sip.getAttributeWr<carb::flatcache::Token>(addPathFabric, FabricTokens::info_sourceAsset_subIdentifier);
        auto parametersFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(addPathFabric, FabricTokens::_parameters);
        // clang-format on

        *bFabric = pxr::GfVec2f(static_cast<float>(uvTranslation.x), static_cast<float>(uvTranslation.y));
        *infoIdFabric = FabricTokens::nvidia_support_definitions_mdl;
        *infoSourceAssetSubIdentifierFabric = FabricTokens::add_float2_float2;
        parametersFabric[0] = FabricTokens::b;
    }

    // lookup_color
    {
        const auto lookupColorPathFabric = carb::flatcache::Path(carb::flatcache::asInt(lookupColorPath));
        sip.createPrim(lookupColorPathFabric);

        FabricAttributesBuilder attributes;

        // clang-format off
        attributes.addAttribute(FabricTypes::wrap_u, FabricTokens::wrap_u);
        attributes.addAttribute(FabricTypes::wrap_v, FabricTokens::wrap_v);
        attributes.addAttribute(FabricTypes::tex, FabricTokens::tex);
        attributes.addAttribute(FabricTypes::info_id, FabricTokens::info_id);
        attributes.addAttribute(FabricTypes::info_sourceAsset_subIdentifier, FabricTokens::info_sourceAsset_subIdentifier);
        attributes.addAttribute(FabricTypes::_paramColorSpace, FabricTokens::_paramColorSpace);
        attributes.addAttribute(FabricTypes::_parameters, FabricTokens::_parameters);
        attributes.addAttribute(FabricTypes::Shader, FabricTokens::Shader);
        attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
        attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
        // clang-format on

        attributes.createAttributes(lookupColorPathFabric);

        sip.setArrayAttributeSize(lookupColorPathFabric, FabricTokens::_paramColorSpace, 2);
        sip.setArrayAttributeSize(lookupColorPathFabric, FabricTokens::_parameters, 3);

        // clang-format off
        auto wrapUFabric = sip.getAttributeWr<int>(lookupColorPathFabric, FabricTokens::wrap_u);
        auto wrapVFabric = sip.getAttributeWr<int>(lookupColorPathFabric, FabricTokens::wrap_v);
        auto texFabric = sip.getAttributeWr<FabricAsset>(lookupColorPathFabric, FabricTokens::tex);
        auto infoIdFabric = sip.getAttributeWr<carb::flatcache::Token>(lookupColorPathFabric, FabricTokens::info_id);
        auto infoSourceAssetSubIdentifierFabric = sip.getAttributeWr<carb::flatcache::Token>(lookupColorPathFabric, FabricTokens::info_sourceAsset_subIdentifier);
        auto paramColorSpaceFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(lookupColorPathFabric, FabricTokens::_paramColorSpace);
        auto parametersFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(lookupColorPathFabric, FabricTokens::_parameters);
        // clang-format on

        *wrapUFabric = 0;
        *wrapVFabric = 0;
        *texFabric = FabricAsset(textureAssetPath);
        *infoIdFabric = FabricTokens::nvidia_support_definitions_mdl;
        *infoSourceAssetSubIdentifierFabric = FabricTokens::lookup_color;
        paramColorSpaceFabric[0] = FabricTokens::tex;
        paramColorSpaceFabric[1] = FabricTokens::_auto;
        parametersFabric[0] = FabricTokens::tex;
        parametersFabric[1] = FabricTokens::wrap_u;
        parametersFabric[2] = FabricTokens::wrap_v;
    }
}

} // namespace cesium::omniverse
