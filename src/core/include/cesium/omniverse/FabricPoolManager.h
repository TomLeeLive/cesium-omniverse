#pragma once

#include "cesium/omniverse/FabricMesh.h"

namespace cesium::omniverse {

class FabricGeometry;
class FabricGeometryPool;
class FabricMaterialPool;

class FabricGeometryDefinition;
class FabricMaterialDefinition;

class FabricPoolManager {
  public:
    FabricPoolManager(const FabricPoolManager&) = delete;
    FabricPoolManager(FabricPoolManager&&) = delete;
    FabricPoolManager& operator=(const FabricPoolManager&) = delete;
    FabricPoolManager& operator=(FabricPoolManager) = delete;

    static FabricPoolManager& getInstance() {
        static FabricPoolManager instance;
        return instance;
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
        uint64_t imageryUvSetIndex);

    void releaseMesh(FabricMesh mesh);

  protected:
    FabricPoolManager() = default;
    ~FabricPoolManager() = default;

  private:
    std::shared_ptr<FabricGeometryPool> getGeometryPool(const FabricGeometryDefinition& geometryDefinition);
    std::shared_ptr<FabricMaterialPool> getMaterialPool(const FabricMaterialDefinition& materialDefinition);

    std::vector<std::shared_ptr<FabricGeometryPool>> _geometryPools;
    std::vector<std::shared_ptr<FabricMaterialPool>> _materialPools;
};

} // namespace cesium::omniverse
