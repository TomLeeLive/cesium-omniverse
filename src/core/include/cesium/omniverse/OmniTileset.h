#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Cesium3DTilesSelection {
class Tileset;
class ViewState;
} // namespace Cesium3DTilesSelection

namespace cesium::omniverse {
class RenderResourcesPreparer;

class OmniTileset {
  public:
    OmniTileset(const std::string& name, long stageId, const std::string& url);
    OmniTileset(const std::string& name, long stageId, int64_t ionId, const std::string& ionToken);
    ~OmniTileset();
    void updateFrame(const std::vector<Cesium3DTilesSelection::ViewState>& viewStates);
    void addIonRasterOverlay(const std::string& name, int64_t ionId, const std::string& ionToken);

  private:
    std::unique_ptr<Cesium3DTilesSelection::Tileset> _tileset;
    std::shared_ptr<RenderResourcesPreparer> _renderResourcesPreparer;
};
} // namespace cesium::omniverse
