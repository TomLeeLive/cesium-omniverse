#pragma once

#include <omni/ui/ImageProvider/DynamicTextureProvider.h>
#include <pxr/usd/sdf/path.h>

namespace cesium::omniverse {

class FabricAttributesBuilder;

class FabricPrim {
  public:
    FabricPrim(pxr::SdfPath path, const FabricAttributesBuilder& attributes);
    ~FabricPrim();
    void setActive(bool active);
    pxr::SdfPath getPath() const;
    const FabricAttributesBuilder& getAttributes() const;

  private:
    pxr::SdfPath _geomPath;
    std::vector<pxr::SdfPath> _materialsPaths;
    std::vector<omni::ui::DynamicTextureProvider> _textures;

    const FabricAttributesBuilder& _attributes;
};

} // namespace cesium::omniverse
