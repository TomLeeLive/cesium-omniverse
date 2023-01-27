#pragma once

#include <CesiumGeospatial/Cartographic.h>
#include <glm/glm.hpp>
#include <spdlog/logger.h>

#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
namespace Cesium3DTilesSelection {
class CreditSystem;
class ViewState;
} // namespace Cesium3DTilesSelection

namespace CesiumGeospatial {
class Cartographic;
}

namespace cesium::omniverse {

class HttpAssetAccessor;
class OmniTileset;
class TaskProcessor;

class Context {
  public:
    static Context& instance();

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
    std::shared_ptr<spdlog::logger> getLogger();

    int addTilesetUrl(long stageId, const std::string& url);
    int addTilesetIon(long stageId, int64_t ionId, const std::string& accessToken);
    void removeTileset(int tileset);
    void addIonRasterOverlay(int tileset, const std::string& name, int64_t ionId, const std::string& ionToken);
    void
    updateFrame(long stageId, const glm::dmat4& viewMatrix, const glm::dmat4& projMatrix, double width, double height);
    void setGeoreferenceOrigin(const CesiumGeospatial::Cartographic& origin);

  private:
    Context() = default;

    int getTilesetId();

    std::shared_ptr<TaskProcessor> _taskProcessor;
    std::shared_ptr<HttpAssetAccessor> _httpAssetAccessor;
    std::shared_ptr<Cesium3DTilesSelection::CreditSystem> _creditSystem;
    std::shared_ptr<spdlog::logger> _logger;
    std::unordered_map<int, std::unique_ptr<OmniTileset>> _tilesets;
    std::vector<Cesium3DTilesSelection::ViewState> _viewStates;
    int _tilesetIdCount = 0;
    std::filesystem::path _cesiumMemLocation;
    std::filesystem::path _certificatePath;

    CesiumGeospatial::Cartographic _georeferenceOrigin{0.0, 0.0, 0.0};
};

#define CESIUM_LOG_VERBOSE(message, ...) Context::instance().getLogger()->verbose(message, __VA_ARGS__);
#define CESIUM_LOG_INFO(message, ...) Context::instance().getLogger()->info(message, __VA_ARGS__);
#define CESIUM_LOG_WARN(message, ...) Context::instance().getLogger()->warn(message, __VA_ARGS__);
#define CESIUM_LOG_ERROR(message, ...) Context::instance().getLogger()->error(message, __VA_ARGS__);
#define CESIUM_LOG_FATAL(message, ...) Context::instance().getLogger()->fatal(message, __VA_ARGS__);

} // namespace cesium::omniverse
