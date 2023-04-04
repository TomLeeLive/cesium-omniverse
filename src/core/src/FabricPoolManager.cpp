#include "cesium/omniverse/FabricPoolManager.h"

#include "cesium/omniverse/FabricGeometry.h"
#include "cesium/omniverse/FabricGeometryDefinition.h"
#include "cesium/omniverse/FabricGeometryPool.h"
#include "cesium/omniverse/FabricMaterial.h"
#include "cesium/omniverse/FabricMaterialDefinition.h"
#include "cesium/omniverse/FabricMaterialPool.h"
#include "cesium/omniverse/GltfUtil.h"

namespace cesium::omniverse {

FabricMesh FabricPoolManager::acquireMesh(
    int64_t tilesetId,
    int64_t tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    bool smoothNormals) {

    // Create the geometry definition (this helps select the right pool)
    FabricGeometryDefinition geometryDefinition;
    geometryDefinition.initializeFromGltf(model, primitive);

    // Create the material definition (this helps select the right pool)

    const auto hasMaterial = GltfUtil::hasMaterial(model, primitive);
    // Acquire a geometry from the pool
    const auto geometryPool = getGeometryPool(geometryDefinition);
    const auto geometry = geometryPool->acquire();

    geometry->initializeFromGltf(
        tilesetId, tileId, ecefToUsdTransform, gltfToEcefTransform, nodeTransform, model, primitive, 0, smoothNormals);

    if (hasMaterial) {
        FabricMaterialDefinition materialDefinition;
        materialDefinition.initializeFromGltf(model, primitive);

        const auto materialPool = getMaterialPool(materialDefinition);
        const auto material = materialPool->acquire();

        geometry->assignMaterial(material);
    }

    return FabricMesh(geometry, geometryDefinition);
}

FabricMesh FabricPoolManager::acquireMesh(
    int64_t tilesetId,
    int64_t tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    bool smoothNormals,
    CesiumGltf::ImageCesium* const imagery,
    const glm::dvec2& imageryUvTranslation,
    const glm::dvec2& imageryUvScale,
    uint64_t imageryUvSetIndex) {

    const auto hasImagery = imagery != nullptr;

    FabricGeometryDefinition geometryDefinition(model, primitive, smoothNormals, hasImagery);
    FabricMaterialDefinition materialDefinition(model, primitive, hasImagery, imageryUvTranslation, imageryUvScale);

    // Acquire a geometry from the pool, using geometryDefinition to select the right pool
    const auto geometryPool = getGeometryPool(geometryDefinition);
    const auto geometry = geometryPool->acquire();

    geometry->initializeFromGltf(
        tilesetId,
        tileId,
        ecefToUsdTransform,
        gltfToEcefTransform,
        nodeTransform,
        model,
        primitive,
        imageryUvSetIndex,
        smoothNormals);

    if (materialDefinition.getHasMaterial()) {
        // Acquire a material from the pool, using materialDefinition to select the right pool
        const auto materialPool = getMaterialPool(materialDefinition);
        const auto material = materialPool->acquire();

        material->initializeFromGltf();

        geometry->assignMaterial(material);

        return FabricMesh(geometry, geometryDefinition);
    }

    return FabricMesh(geometry, geometryDefinition);
}

void FabricPoolManager::releaseMesh(FabricMesh mesh) {
    const auto geometryPool = getGeometryPool(mesh.getGeometryDefinition());
    geometryPool->release(mesh.getGeometry());

    const auto materialPool = getMaterialPool(mesh.getMaterialDefinition());
    materialPool->release(mesh.getMaterial());
}

std::shared_ptr<FabricGeometryPool>
FabricPoolManager::getGeometryPool(const FabricGeometryDefinition& geometryDefinition) {
    // Find a pool with the same geometry definition
    for (const auto geometryPool : _geometryPools) {
        if (geometryDefinition == geometryPool->getGeometryDefinition()) {
            return geometryPool;
        }
    }

    // Otherwise, create a new pool
    const auto id = _geometryPools.size();
    return _geometryPools.emplace_back(std::make_shared<FabricGeometryPool>(id, geometryDefinition));
}

std::shared_ptr<FabricMaterialPool>
FabricPoolManager::getMaterialPool(const FabricMaterialDefinition& materialDefinition) {
    // Find a pool with the same material definition
    for (const auto materialPool : _materialPools) {
        if (materialDefinition == materialPool->getMaterialDefinition()) {
            return materialPool;
        }
    }

    // Otherwise, create a new pool
    const auto id = _materialPools.size();
    return _materialPools.emplace_back(std::make_shared<FabricGeometryPool>(id, materialDefinition));
}

}; // namespace cesium::omniverse
