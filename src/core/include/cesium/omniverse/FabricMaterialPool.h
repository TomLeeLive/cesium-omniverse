#pragma once

#include <pxr/usd/sdf/path.h>

#include <list>

namespace cesium::omniverse {

class FabricMaterialPoolItem;

class FabricMaterialPool {
  public:
    FabricMaterialPool();

    void initialize();

    pxr::SdfPath get();
    void release(const pxr::SdfPath& path);

  private:
    std::vector<FabricMaterialPoolItem> _items;
};

class FabricMaterialPoolItem {
  public:
    FabricMaterialPoolItem(const pxr::SdfPath& materialPath, const std::string& textureName);

  private:
    pxr::SdfPath _path;
    bool _occupied;

    friend FabricMaterialPool;
};

} // namespace cesium::omniverse
