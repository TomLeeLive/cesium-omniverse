#pragma once

#include <glm/glm.hpp>

namespace CesiumGltf {
struct MeshPrimitive;
struct Model;
} // namespace CesiumGltf

namespace cesium::omniverse {

class FabricMaterialDefinition {
  public:
    FabricMaterialDefinition(
        const CesiumGltf::Model& model,
        const CesiumGltf::MeshPrimitive& primitive,
        bool hasImagery,
        const glm::dvec2& imageryUvTranslation,
        const glm::dvec2& imageryUvScale);

    glm::dvec3 getBaseColorFactor() const;
    double getMetallicFactor() const;
    double getRoughnessFactor() const;
    glm::dvec2 getUvTranslation() const;
    glm::dvec2 getUvScale() const;
    bool getHasMaterial() const;
    bool getHasBaseColorTexture() const;
    bool getHasUvTransform() const;

    bool operator==(const FabricMaterialDefinition& other) const;

  private:
    // In future versions of Kit where material values can be set dynamically we won't need any of these properties
    // in the material definition except hasBaseColorTexture and hasUvTransform
    glm::dvec3 _baseColorFactor{1.0, 1.0, 1.0};
    double _metallicFactor{0.0};
    double _roughnessFactor{1.0};
    glm::dvec2 _uvTranslation{0.0, 0.0};
    glm::dvec2 _uvScale{1.0, 1.0};

    bool _hasMaterial{false};
    bool _hasBaseColorTexture{false};
    bool _hasUvTransform{false};
};

} // namespace cesium::omniverse
