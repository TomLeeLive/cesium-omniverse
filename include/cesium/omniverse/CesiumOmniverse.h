#pragma once

#include <carb/Interface.h>
#include <pxr/pxr.h>

#include <cstdint>

PXR_NAMESPACE_OPEN_SCOPE
class GfMatrix4d;
PXR_NAMESPACE_CLOSE_SCOPE

namespace cesium::omniverse {

class ICesiumOmniverseInterface {
  public:
    CARB_PLUGIN_INTERFACE("cesium::omniverse::ICesiumOmniverseInterface", 0, 0);

    /**
     * @brief Call this before any tilesets are created.
     *
     * @param cesiumExtensionLocation The extension location
     */
    virtual void init(const char* cesiumExtensionLocation) noexcept = 0;

    /**
     * @brief Call this to free resources on program exit.
     */
    virtual void destroy() noexcept = 0;

    /**
     * @brief Adds a Cesium Data prim.
     *
     * @param stageId The USD stage id
     * @param ionToken The Stage level ionToken
     */
    virtual void addCesiumData(long stageId, const char* ionToken) noexcept = 0;

    /**
     * @brief Adds a tileset from url.
     *
     * @param stageId The USD stage id
     * @param url The tileset url
     * @returns The tileset id. Returns -1 on error.
     */
    virtual int addTilesetUrl(long stageId, const char* url) noexcept = 0;

    /**
     * @brief Adds a tileset from ion using the Stage level ion token.
     *
     * @param stageId The USD stage id
     * @param ionId The ion asset id
     * @param ionToken The access token
     * @returns The tileset id. Returns -1 on error.
     */
    virtual int addTilesetIon(long stageId, int64_t ionId, const char* ionToken) noexcept = 0;

    /**
     * @brief Removes a tileset from the scene.
     *
     * @param tileset The tileset id. If there's no tileset with this id nothing happens.
     */
    virtual void removeTileset(int tileset) noexcept = 0;

    /**
     * @brief Adds a raster overlay from ion.
     *
     * @param tileset The tileset id
     * @param name The user-given name of this overlay layer
     * @param ionId The asset ID
     * @param ionToken The access token
     */
    virtual void addIonRasterOverlay(int tileset, const char* name, int64_t ionId, const char* ionToken) noexcept = 0;

    /**
     * @brief Updates all tilesets this frame.
     *
     * @param viewMatrix The view matrix
     * @param projMatrix The projection matrix
     * @param width The screen width
     * @param height The screen height
     */
    virtual void updateFrame(
        // TODO: take a double* instead
        const pxr::GfMatrix4d& viewMatrix,
        const pxr::GfMatrix4d& projMatrix,
        double width,
        double height) noexcept = 0;

    /**
     * @brief Sets the georeference origin based on the WGS84 ellipsoid.
     *
     * @param stageId The USD stage id
     * @param longitude The longitude in degrees
     * @param latitude The latitude in degrees
     * @param height The height in meters
     */
    virtual void setGeoreferenceOrigin(long stageId, double longitude, double latitude, double height) noexcept = 0;

    /**
     * @brief Adds a cube to Fabric using the USDRT API.
     *
     * @param stageId The USD stage id
     */
    virtual void addCubeUsdrt(long stageId) noexcept = 0;

    /**
     * @brief Adds a cube to the USD stage.
     *
     * @param stageId The USD stage id
     */
    virtual void addCubeUsd(long stageId) noexcept = 0;

    /**
     * @brief Adds a cube to Fabric directly.
     *
     * @param stageId The USD stage id
     */
    virtual void addCubeFabric(long stageId) noexcept = 0;

    /**
     * @brief Remove the USDRT cube.
     *
     * @param stageId The USD stage id
     */
    virtual void removeCubeUsdrt(long stageId) noexcept = 0;

    /**
     * @brief Print the USDRT stage.
     *
     * @param stageId The USD stage id
     * @returns A string representation of the USDRT stage.
     */
    virtual std::string printUsdrtStage(long stageId) noexcept = 0;

    /**
     * @brief Print the Fabric stage.
     *
     * @param stageId The USD stage id
     * @returns A string representation of the Fabric stage.
     */
    virtual std::string printFabricStage(long stageId) noexcept = 0;
};

} // namespace cesium::omniverse
