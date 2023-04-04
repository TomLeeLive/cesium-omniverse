#pragma once

namespace CesiumGltf {
struct MeshPrimitive;
struct Model;
} // namespace CesiumGltf

namespace cesium::omniverse {

class FabricGeometryDefinition {
  public:
    FabricGeometryDefinition(
        const CesiumGltf::Model& model,
        const CesiumGltf::MeshPrimitive& primitive,
        bool smoothNormals,
        bool hasImagery);

    bool getHasMaterial() const;
    bool getHasSt() const;
    bool getHasNormals() const;

    bool operator==(const FabricGeometryDefinition& other) const;

  private:
    bool _hasMaterial{false};
    bool _hasSt{false};
    bool _hasNormals{false};
};

} // namespace cesium::omniverse
