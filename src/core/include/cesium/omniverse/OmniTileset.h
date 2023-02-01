#pragma once

#include <glm/glm.hpp>
#include <pxr/usd/sdf/path.h>

#include <memory>
#include <string>
#include <vector>

namespace Cesium3DTilesSelection {
class Tileset;
class ViewState;
} // namespace Cesium3DTilesSelection

namespace cesium::omniverse {
class RenderResourcesPreparer;

struct OmniTilesetFrameState {
    glm::dmat4 ecefToUsdTransform;
    bool visible = true;
};

class OmniTileset {
  public:
    OmniTileset(long stageId, int tilesetId, const pxr::SdfPath& usdPath, const std::string& url);
    OmniTileset(long stageId, int tilesetId, const pxr::SdfPath& usdPath, int64_t ionId, const std::string& ionToken);
    ~OmniTileset();
    void updateFrame(const std::vector<Cesium3DTilesSelection::ViewState>& viewStates);
    void addIonRasterOverlay(const std::string& name, int64_t ionId, const std::string& ionToken);
    const pxr::SdfPath& getUsdPath() const;
    const int getId() const;
    OmniTilesetFrameState& getFrameState();
    bool getSuspendUpdate() const;
    void setSuspendUpdate(bool suspendUpdate);


  private:
    std::unique_ptr<Cesium3DTilesSelection::Tileset> _tileset;
    std::shared_ptr<RenderResourcesPreparer> _renderResourcesPreparer;
    pxr::SdfPath _usdPath;
    int _id;
    bool _suspendUpdate = false;
    OmniTilesetFrameState _frameState;
};
} // namespace cesium::omniverse
