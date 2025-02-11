#pragma once

#include "cesium/omniverse/GltfUtil.h"

#include <cstdint>
#include <set>

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
        const FeaturesInfo& featuresInfo,
        bool smoothNormals);

    [[nodiscard]] bool hasNormals() const;
    [[nodiscard]] bool hasVertexColors() const;
    [[nodiscard]] bool hasVertexIds() const;
    [[nodiscard]] uint64_t getTexcoordSetCount() const;
    [[nodiscard]] const std::set<VertexAttributeInfo>& getCustomVertexAttributes() const;

    bool operator==(const FabricGeometryDefinition& other) const;

  private:
    bool _hasNormals{false};
    bool _hasVertexColors{false};
    bool _hasVertexIds{false};
    uint64_t _texcoordSetCount{0};

    // std::set is sorted which is important for checking FabricGeometryDefinition equality
    // Note that features ids are treated as custom vertex attributes since they don't have specific parsing behavior
    std::set<VertexAttributeInfo> _customVertexAttributes;
};

} // namespace cesium::omniverse
