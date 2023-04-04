#pragma once

#include <omni/ui/ImageProvider/DynamicTextureProvider.h>
#include <pxr/usd/sdf/path.h>

namespace cesium::omniverse {

class FabricMaterial {
  public:
    FabricMaterial(pxr::SdfPath path);
    ~FabricMaterial();
    void setActive(bool active);
    const std::vector<pxr::SdfPath>& getPaths() const;

    void initializeFromGltf(tilesetId,
        tileId,
        ecefToUsdTransform,
        gltfToEcefTransform,
        nodeTransform,
        model,
        primitive,
        imageryUvSetIndex,
        smoothNormals

  private:
    std::vector<pxr::SdfPath> _paths;
    std::vector<omni::ui::DynamicTextureProvider> _textures;

    const FabricMaterialDefinition& _materialDefinition;
};

} // namespace cesium::omniverse
