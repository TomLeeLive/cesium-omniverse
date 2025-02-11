#pragma once

#include "cesium/omniverse/FabricMaterial.h"
#include "cesium/omniverse/FabricMaterialDefinition.h"
#include "cesium/omniverse/ObjectPool.h"

#include <pxr/usd/sdf/assetPath.h>

namespace cesium::omniverse {

class FabricMaterialPool final : public ObjectPool<FabricMaterial> {
  public:
    FabricMaterialPool(
        int64_t poolId,
        const FabricMaterialDefinition& materialDefinition,
        uint64_t initialCapacity,
        const pxr::TfToken& defaultTextureAssetPathToken,
        const pxr::TfToken& defaultTransparentTextureAssetPathToken,
        bool debugRandomColors,
        long stageId);

    [[nodiscard]] const FabricMaterialDefinition& getMaterialDefinition() const;
    void updateShaderInput(const pxr::SdfPath& shaderPath, const pxr::TfToken& attributeName);

  protected:
    std::shared_ptr<FabricMaterial> createObject(uint64_t objectId) override;
    void setActive(std::shared_ptr<FabricMaterial> material, bool active) override;

  private:
    const int64_t _poolId;
    const FabricMaterialDefinition _materialDefinition;
    const pxr::TfToken _defaultTextureAssetPathToken;
    const pxr::TfToken _defaultTransparentTextureAssetPathToken;
    const bool _debugRandomColors;
    const long _stageId;
};

} // namespace cesium::omniverse
