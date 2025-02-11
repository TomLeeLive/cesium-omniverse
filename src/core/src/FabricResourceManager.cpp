#include "cesium/omniverse/FabricResourceManager.h"

#include "cesium/omniverse/FabricGeometry.h"
#include "cesium/omniverse/FabricGeometryDefinition.h"
#include "cesium/omniverse/FabricGeometryPool.h"
#include "cesium/omniverse/FabricMaterialDefinition.h"
#include "cesium/omniverse/FabricMaterialPool.h"
#include "cesium/omniverse/FabricTexture.h"
#include "cesium/omniverse/FabricTexturePool.h"
#include "cesium/omniverse/FabricUtil.h"
#include "cesium/omniverse/GltfUtil.h"
#include "cesium/omniverse/UsdUtil.h"

#include <omni/ui/ImageProvider/DynamicTextureProvider.h>
#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse {

namespace {
template <typename T> void removePool(std::vector<T>& pools, const T& pool) {
    pools.erase(
        std::remove_if(pools.begin(), pools.end(), [&pool](const auto& other) { return pool.get() == other.get(); }),
        pools.end());
}

std::unique_ptr<omni::ui::DynamicTextureProvider>
createSinglePixelTexture(const std::string& name, std::array<uint8_t, 4> bytes) {
    const auto size = carb::Uint2{1, 1};
    auto defaultTexture = std::make_unique<omni::ui::DynamicTextureProvider>(name);
    defaultTexture->setBytesData(bytes.data(), size, omni::ui::kAutoCalculateStride, carb::Format::eRGBA8_SRGB);
    return defaultTexture;
}

const std::string DEFAULT_TEXTURE_NAME = "fabric_default_texture";
const std::string DEFAULT_TRANSPARENT_TEXTURE_NAME = "fabric_default_transparent_texture";

} // namespace

FabricResourceManager::FabricResourceManager() {
    _defaultTexture = createSinglePixelTexture(DEFAULT_TEXTURE_NAME, {{255, 255, 255, 255}});
    _defaultTextureAssetPathToken = UsdUtil::getDynamicTextureProviderAssetPathToken(DEFAULT_TEXTURE_NAME);

    _defaultTransparentTexture = createSinglePixelTexture(DEFAULT_TRANSPARENT_TEXTURE_NAME, {{255, 255, 255, 0}});
    _defaultTransparentTextureAssetPathToken =
        UsdUtil::getDynamicTextureProviderAssetPathToken(DEFAULT_TRANSPARENT_TEXTURE_NAME);
}

FabricResourceManager::~FabricResourceManager() = default;

bool FabricResourceManager::shouldAcquireMaterial(
    const CesiumGltf::MeshPrimitive& primitive,
    bool hasImagery,
    const pxr::SdfPath& tilesetMaterialPath) const {
    if (_disableMaterials) {
        return false;
    }

    if (!tilesetMaterialPath.IsEmpty()) {
        return FabricUtil::materialHasCesiumNodes(FabricUtil::toFabricPath(tilesetMaterialPath));
    }

    return hasImagery || GltfUtil::hasMaterial(primitive);
}

std::shared_ptr<FabricGeometry> FabricResourceManager::acquireGeometry(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const FeaturesInfo& featuresInfo,
    bool smoothNormals,
    long stageId) {

    FabricGeometryDefinition geometryDefinition(model, primitive, featuresInfo, smoothNormals);

    if (_disableGeometryPool) {
        const auto pathStr = fmt::format("/fabric_geometry_{}", getNextGeometryId());
        const auto path = omni::fabric::Path(pathStr.c_str());
        return std::make_shared<FabricGeometry>(path, geometryDefinition, stageId);
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    auto geometryPool = getGeometryPool(geometryDefinition);

    if (geometryPool == nullptr) {
        geometryPool = createGeometryPool(geometryDefinition, stageId);
    }

    auto geometry = geometryPool->acquire();

    return geometry;
}

bool useSharedMaterial(const FabricMaterialDefinition& materialDefinition) {
    if (materialDefinition.hasBaseColorTexture() || materialDefinition.getImageryLayerCount() > 0 ||
        !materialDefinition.getFeatureIdTypes().empty() || !materialDefinition.getProperties().empty()) {
        return false;
    }

    return true;
}

std::shared_ptr<FabricMaterial>
FabricResourceManager::createMaterial(const FabricMaterialDefinition& materialDefinition, long stageId) {
    const auto pathStr = fmt::format("/fabric_material_{}", getNextMaterialId());
    const auto path = omni::fabric::Path(pathStr.c_str());
    return std::make_shared<FabricMaterial>(
        path,
        materialDefinition,
        _defaultTextureAssetPathToken,
        _defaultTransparentTextureAssetPathToken,
        _debugRandomColors,
        stageId);
}

void FabricResourceManager::removeSharedMaterial(const SharedMaterial& sharedMaterial) {
    _sharedMaterials.erase(
        std::remove_if(
            _sharedMaterials.begin(),
            _sharedMaterials.end(),
            [&sharedMaterial](const auto& other) { return &sharedMaterial == &other; }),
        _sharedMaterials.end());
}

SharedMaterial* FabricResourceManager::getSharedMaterial(const MaterialInfo& materialInfo, int64_t tilesetId) {
    for (auto& sharedMaterial : _sharedMaterials) {
        if (sharedMaterial.materialInfo == materialInfo && sharedMaterial.tilesetId == tilesetId) {
            return &sharedMaterial;
        }
    }

    return nullptr;
}

SharedMaterial* FabricResourceManager::getSharedMaterial(const std::shared_ptr<FabricMaterial>& material) {
    for (auto& sharedMaterial : _sharedMaterials) {
        if (sharedMaterial.material.get() == material.get()) {
            return &sharedMaterial;
        }
    }

    return nullptr;
}

std::shared_ptr<FabricMaterial> FabricResourceManager::acquireSharedMaterial(
    const MaterialInfo& materialInfo,
    const FabricMaterialDefinition& materialDefinition,
    long stageId,
    int64_t tilesetId) {

    const auto sharedMaterial = getSharedMaterial(materialInfo, tilesetId);

    if (sharedMaterial != nullptr) {
        sharedMaterial->referenceCount++;
        return sharedMaterial->material;
    }

    auto material = createMaterial(materialDefinition, stageId);

    _sharedMaterials.emplace_back(SharedMaterial{
        material,
        materialInfo,
        tilesetId,
        1,
    });

    return material;
}

void FabricResourceManager::releaseSharedMaterial(const std::shared_ptr<FabricMaterial>& material) {
    const auto sharedMaterial = getSharedMaterial(material);

    assert(sharedMaterial != nullptr);

    if (sharedMaterial != nullptr) {
        sharedMaterial->referenceCount--;
        if (sharedMaterial->referenceCount == 0) {
            removeSharedMaterial(*sharedMaterial);
        }
    }
}

std::shared_ptr<FabricMaterial> FabricResourceManager::acquireMaterial(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    const MaterialInfo& materialInfo,
    const FeaturesInfo& featuresInfo,
    uint64_t imageryLayerCount,
    long stageId,
    int64_t tilesetId,
    const pxr::SdfPath& tilesetMaterialPath) {
    FabricMaterialDefinition materialDefinition(
        model, primitive, materialInfo, featuresInfo, imageryLayerCount, _disableTextures, tilesetMaterialPath);

    if (useSharedMaterial(materialDefinition)) {
        return acquireSharedMaterial(materialInfo, materialDefinition, stageId, tilesetId);
    }

    if (_disableMaterialPool) {
        return createMaterial(materialDefinition, stageId);
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    auto materialPool = getMaterialPool(materialDefinition);

    if (materialPool == nullptr) {
        materialPool = createMaterialPool(materialDefinition, stageId);
    }

    auto material = materialPool->acquire();

    return material;
}

std::shared_ptr<FabricTexture> FabricResourceManager::acquireTexture() {
    if (_disableTexturePool) {
        const auto name = fmt::format("/fabric_texture_{}", getNextTextureId());
        return std::make_shared<FabricTexture>(name);
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    auto texturePool = getTexturePool();

    if (texturePool == nullptr) {
        texturePool = createTexturePool();
    }

    auto texture = texturePool->acquire();

    return texture;
}

void FabricResourceManager::releaseGeometry(const std::shared_ptr<FabricGeometry>& geometry) {
    if (_disableGeometryPool) {
        return;
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    const auto geometryPool = getGeometryPool(geometry->getGeometryDefinition());
    assert(geometryPool != nullptr);
    geometryPool->release(geometry);
}

void FabricResourceManager::releaseMaterial(const std::shared_ptr<FabricMaterial>& material) {
    const auto& materialDefinition = material->getMaterialDefinition();

    if (useSharedMaterial(materialDefinition)) {
        releaseSharedMaterial(material);
        return;
    }

    if (_disableMaterialPool) {
        return;
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    const auto materialPool = getMaterialPool(materialDefinition);
    assert(materialPool != nullptr);
    materialPool->release(material);
}

void FabricResourceManager::releaseTexture(const std::shared_ptr<FabricTexture>& texture) {
    if (_disableTexturePool) {
        return;
    }

    std::scoped_lock<std::mutex> lock(_poolMutex);

    const auto texturePool = getTexturePool();
    assert(texturePool != nullptr);
    texturePool->release(texture);
}

void FabricResourceManager::setDisableMaterials(bool disableMaterials) {
    _disableMaterials = disableMaterials;
}

void FabricResourceManager::setDisableTextures(bool disableTextures) {
    _disableTextures = disableTextures;
}

void FabricResourceManager::setDisableGeometryPool(bool disableGeometryPool) {
    assert(_geometryPools.size() == 0);
    _disableGeometryPool = disableGeometryPool;
}

void FabricResourceManager::setDisableMaterialPool(bool disableMaterialPool) {
    assert(_materialPools.size() == 0);
    _disableMaterialPool = disableMaterialPool;
}

void FabricResourceManager::setDisableTexturePool(bool disableTexturePool) {
    assert(_texturePools.size() == 0);
    _disableTexturePool = disableTexturePool;
}

void FabricResourceManager::setGeometryPoolInitialCapacity(uint64_t geometryPoolInitialCapacity) {
    assert(_geometryPools.size() == 0);
    _geometryPoolInitialCapacity = geometryPoolInitialCapacity;
}

void FabricResourceManager::setMaterialPoolInitialCapacity(uint64_t materialPoolInitialCapacity) {
    assert(_materialPools.size() == 0);
    _materialPoolInitialCapacity = materialPoolInitialCapacity;
}

void FabricResourceManager::setTexturePoolInitialCapacity(uint64_t texturePoolInitialCapacity) {
    assert(_texturePools.size() == 0);
    _texturePoolInitialCapacity = texturePoolInitialCapacity;
}

void FabricResourceManager::setDebugRandomColors(bool debugRandomColors) {
    _debugRandomColors = debugRandomColors;
}

void FabricResourceManager::updateShaderInput(
    const pxr::SdfPath& materialPath,
    const pxr::SdfPath& shaderPath,
    const pxr::TfToken& attributeName) {
    for (auto& materialPool : _materialPools) {
        const auto& tilesetMaterialPath = materialPool->getMaterialDefinition().getTilesetMaterialPath();
        if (tilesetMaterialPath == materialPath) {
            materialPool->updateShaderInput(shaderPath, attributeName);
        }
    }
}

void FabricResourceManager::clear() {
    _geometryPools.clear();
    _materialPools.clear();
    _texturePools.clear();
    _sharedMaterials.clear();
}

std::shared_ptr<FabricGeometryPool>
FabricResourceManager::getGeometryPool(const FabricGeometryDefinition& geometryDefinition) {
    for (const auto& geometryPool : _geometryPools) {
        if (geometryDefinition == geometryPool->getGeometryDefinition()) {
            // Found a pool with the same geometry definition
            return geometryPool;
        }
    }

    return nullptr;
}

std::shared_ptr<FabricMaterialPool>
FabricResourceManager::getMaterialPool(const FabricMaterialDefinition& materialDefinition) {
    for (const auto& materialPool : _materialPools) {
        if (materialDefinition == materialPool->getMaterialDefinition()) {
            // Found a pool with the same material definition
            return materialPool;
        }
    }

    return nullptr;
}

std::shared_ptr<FabricTexturePool> FabricResourceManager::getTexturePool() {
    if (!_texturePools.empty()) {
        return _texturePools.front();
    }

    return nullptr;
}

std::shared_ptr<FabricGeometryPool>
FabricResourceManager::createGeometryPool(const FabricGeometryDefinition& geometryDefinition, long stageId) {
    return _geometryPools.emplace_back(std::make_shared<FabricGeometryPool>(
        getNextPoolId(), geometryDefinition, _geometryPoolInitialCapacity, stageId));
}

std::shared_ptr<FabricMaterialPool>
FabricResourceManager::createMaterialPool(const FabricMaterialDefinition& materialDefinition, long stageId) {
    return _materialPools.emplace_back(std::make_shared<FabricMaterialPool>(
        getNextPoolId(),
        materialDefinition,
        _materialPoolInitialCapacity,
        _defaultTextureAssetPathToken,
        _defaultTransparentTextureAssetPathToken,
        _debugRandomColors,
        stageId));
}

std::shared_ptr<FabricTexturePool> FabricResourceManager::createTexturePool() {
    return _texturePools.emplace_back(
        std::make_shared<FabricTexturePool>(getNextPoolId(), _texturePoolInitialCapacity));
}

void FabricResourceManager::retainPath(const omni::fabric::Path& path) {
    // This is a workaround for https://github.com/CesiumGS/cesium-omniverse/issues/425.
    // By retaining a reference to the path we avoid a crash deep in the Sdf library
    // that's triggered by loading a tileset, removing the tileset, and reloading the tileset.
    // It's possible this will be fixed in a future Kit release at which point we can remove
    // this workaround.
    _retainedPaths.push_back(path);
}

int64_t FabricResourceManager::getNextGeometryId() {
    return _geometryId++;
}

int64_t FabricResourceManager::getNextMaterialId() {
    return _materialId++;
}

int64_t FabricResourceManager::getNextTextureId() {
    return _textureId++;
}

int64_t FabricResourceManager::getNextPoolId() {
    return _poolId++;
}

}; // namespace cesium::omniverse
