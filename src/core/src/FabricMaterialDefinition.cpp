#include "cesium/omniverse/FabricMaterialDefinition.h"

#include "cesium/omniverse/GltfUtil.h"

#include <CesiumGltf/Model.h>

namespace cesium::omniverse {

FabricMaterialDefinition::FabricMaterialDefinition(
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    bool hasImagery,
    const glm::dvec2& imageryUvTranslation,
    const glm::dvec2& imageryUvScale) {

    const auto hasMaterial = GltfUtil::hasMaterial(model, primitive);

    if (hasMaterial) {
        const auto& material = model.materials[static_cast<size_t>(primitive.material)];

        _baseColorFactor = GltfUtil::getBaseColorFactor(material);
        _metallicFactor = GltfUtil::getMetallicFactor(material);
        _roughnessFactor = GltfUtil::getRoughnessFactor(material);
        _hasBaseColorTexture = GltfUtil::getBaseColorTextureIndex(model, material).has_value();
    }

    if (hasImagery) {
        _uvTranslation = imageryUvTranslation;
        _uvScale = imageryUvScale;
        _hasBaseColorTexture = true;
        _hasUvTransform = hasImagery;
    }

    _hasMaterial = hasMaterial || hasImagery;
}

glm::dvec3 FabricMaterialDefinition::getBaseColorFactor() const {
    return _baseColorFactor;
}
double FabricMaterialDefinition::getMetallicFactor() const {
    return _metallicFactor;
}
double FabricMaterialDefinition::getRoughnessFactor() const {
    return _roughnessFactor;
}
glm::dvec2 FabricMaterialDefinition::getUvTranslation() const {
    return _uvTranslation;
}
glm::dvec2 FabricMaterialDefinition::getUvScale() const {
    return _uvScale;
}
bool FabricMaterialDefinition::getHasMaterial() const {
    return _hasMaterial;
}
bool FabricMaterialDefinition::getHasBaseColorTexture() const {
    return _hasBaseColorTexture;
}
bool FabricMaterialDefinition::getHasUvTransform() const {
    return _hasUvTransform;
}

bool FabricMaterialDefinition::operator==(const FabricMaterialDefinition& other) const {
    return std::memcmp(this, &other, sizeof(FabricMaterialDefinition)) == 0;
}

} // namespace cesium::omniverse
