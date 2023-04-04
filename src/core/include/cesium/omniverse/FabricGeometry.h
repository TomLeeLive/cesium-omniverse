#pragma once

#include <omni/ui/ImageProvider/DynamicTextureProvider.h>
#include <pxr/usd/sdf/path.h>

namespace CesiumGltf {
struct MeshPrimitive;
struct Model;
} // namespace CesiumGltf

namespace cesium::omniverse {

class FabricMaterial;

class FabricGeometryDefinition;
class FabricGeometryDefinition;

class FabricGeometry {
  public:
    FabricGeometry(pxr::SdfPath path, const FabricGeometryDefinition& geometryDefinition);
    ~FabricGeometry();

    void initializeFromGltf(
        int64_t tilesetId,
        int64_t tileId,
        const glm::dmat4& ecefToUsdTransform,
        const glm::dmat4& gltfToEcefTransform,
        const glm::dmat4& nodeTransform,
        const CesiumGltf::Model& model,
        const CesiumGltf::MeshPrimitive& primitive,
        uint64_t imageryUvSetIndex,
        bool smoothNormals);

    void assignMaterial(std::shared_ptr<FabricMaterial> material);

    void setActive(bool active);

    pxr::SdfPath getPath() const;

  private:
    pxr::SdfPath _path;
};

} // namespace cesium::omniverse
