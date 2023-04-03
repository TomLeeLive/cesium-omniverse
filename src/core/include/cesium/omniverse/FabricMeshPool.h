#pragma once

#include <pxr/usd/sdf/path.h>

#include <list>

namespace cesium::omniverse {

class FabricAttributesBuilder;
class FabricMeshPoolItem;

class FabricMeshPool {
  public:
    FabricMeshPool();

    void initialize(const FabricAttributesBuilder& builder);

    pxr::SdfPath get();
    void release(const pxr::SdfPath& path);

  private:
    std::vector<FabricMeshPoolItem> _items;
};

class FabricMeshPoolItem {
  public:
    FabricMeshPoolItem(const pxr::SdfPath& path, const FabricAttributesBuilder& builder);

  private:
    pxr::SdfPath _path;
    bool _occupied;

    friend FabricMeshPool;
};

} // namespace cesium::omniverse
