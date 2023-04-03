#pragma once

#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/ObjectPool.h"

#include <omni/ui/ImageProvider/DynamicTextureProvider.h>
#include <pxr/usd/sdf/path.h>

namespace cesium::omniverse {

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

class FabricPrimPool : public ObjectPool<FabricPrim> {
  public:
    FabricPrimPool(uint64_t id, const FabricAttributesBuilder& attributes);
    void setActive(std::shared_ptr<FabricPrim> prim, bool active) override;

    const FabricAttributesBuilder& getAttributes() const;

  private:
    const FabricAttributesBuilder _attributes;
    const uint64_t _id;
};

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

    std::shared_ptr<FabricPrim> acquirePrim(const FabricAttributesBuilder& attributes);
    void releasePrim(std::shared_ptr<FabricPrim> prim);

  protected:
    FabricPoolManager() = default;
    ~FabricPoolManager() = default;

  private:
    std::shared_ptr<FabricPrimPool> getPrimPool(const FabricAttributesBuilder& attributes);
    std::vector<std::shared_ptr<FabricPrimPool>> _primPools;
};

} // namespace cesium::omniverse
