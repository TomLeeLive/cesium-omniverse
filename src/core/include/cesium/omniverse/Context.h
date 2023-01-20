#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Cesium3DTilesSelection {
class CreditSystem;
class ViewState;
} // namespace Cesium3DTilesSelection

namespace cesium::omniverse {

class CoordinateSystem;
class HttpAssetAccessor;
class OmniTileset;
class TaskProcessor;

class Context {
  public:
    static Context& instance() const;

    ~Context() = default;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;

    void init(const std::filesystem::path& cesiumExtensionLocation);
    void destroy();

    std::shared_ptr<TaskProcessor> getTaskProcessor();
    std::shared_ptr<HttpAssetAccessor> getHttpAssetAccessor();
    std::shared_ptr<Cesium3DTilesSelection::CreditSystem> getCreditSystem();
    std::shared_ptr<CoordinateSystem> getCoordinateSystem();

    void updateFrame(const glm::dmat4& viewMatrix, const glm::dmat4& projMatrix, double width, double height);

  private:
    Context() = default;

    int getTilesetId();

    std::shared_ptr<TaskProcessor> _taskProcessor;
    std::shared_ptr<HttpAssetAccessor> _httpAssetAccessor;
    std::shared_ptr<Cesium3DTilesSelection::CreditSystem> _creditSystem;
    std::shared_ptr<CoordinateSystem> _coordinateSystem;
    std::unordered_map<int, std::unique_ptr<OmniTileset>> _tilesets;
    std::vector<Cesium3DTilesSelection::ViewState> _viewStates;
    int _tilesetId;
    std::filesystem::path _cesiumMemLocation;
    std::filesystem::path _certificatePath;
};
} // namespace cesium::omniverse
