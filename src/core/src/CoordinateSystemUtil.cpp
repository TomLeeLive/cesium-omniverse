#include "cesium/omniverse/CoordinateSystemUtil.h"

#include "cesium/omniverse/UsdUtil.h"

#include <CesiumGeometry/AxisTransforms.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Transforms.h>
#include <glm/gtc/matrix_transform.hpp>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdGeom/tokens.h>

namespace cesium::omniverse {

namespace {
glm::dmat4 getEastNorthUpToFixedFrame(const CesiumGeospatial::Cartographic& cartographic) {
    const auto cartesian = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(cartographic);
    return CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(cartesian);
}

glm::dmat4 getAxisConversionTransform(long stageId) {
    const auto upAxis = getUsdUpAxis(stageId);

    auto axisConversion = glm::dmat4(1.0);
    if (upAxis == pxr::UsdGeomTokens->y) {
        axisConversion = CesiumGeometry::AxisTransforms::Y_UP_TO_Z_UP;
    }

    return axisConversion;
}

glm::dmat4 getUnitConversionTransform(long stageId) {
    const auto metersPerUnit = getUsdMetersPerUnit(stageId);
    return glm::scale(glm::dmat4(1.0), glm::dvec3(metersPerUnit));
}

} // namespace

glm::dmat4 computeUsdToEcefTransform(long stageId, const CesiumGeospatial::Cartographic& origin) {
    return getEastNorthUpToFixedFrame(origin) * getAxisConversionTransform(stageId) *
           getUnitConversionTransform(stageId);
}

glm::dmat4 computeEcefToUsdTransform(long stageId, const CesiumGeospatial::Cartographic& origin) {
    return glm::inverse(computeUsdToEcefTransform(stageId, origin));
}

glm::dmat4 computeEcefToUsdTransformForPrim(
    long stageId,
    const CesiumGeospatial::Cartographic& origin,
    const pxr::SdfPath& primPath) {
    const auto ecefToUsdTransform = computeEcefToUsdTransform(stageId, origin);
    const auto primUsdWorldTransform = computeUsdWorldTransform(stageId, primPath);
    const auto primEcefToUsdTransform = primUsdWorldTransform * ecefToUsdTransform;
    return primEcefToUsdTransform;
}

Cesium3DTilesSelection::ViewState computeViewState(
    long stageId,
    const CesiumGeospatial::Cartographic& origin,
    const glm::dmat4& viewMatrix,
    const glm::dmat4& projMatrix,
    double width,
    double height) {
    const auto usdToEcef = computeUsdToEcefTransform(stageId, origin);
    auto inverseView = glm::inverse(viewMatrix);
    auto omniCameraUp = glm::dvec3(inverseView[1]);
    auto omniCameraFwd = glm::dvec3(-inverseView[2]);
    auto omniCameraPosition = glm::dvec3(inverseView[3]);
    auto cameraUp = glm::dvec3(usdToEcef * glm::dvec4(omniCameraUp, 0.0));
    auto cameraFwd = glm::dvec3(usdToEcef * glm::dvec4(omniCameraFwd, 0.0));
    auto cameraPosition = glm::dvec3(usdToEcef * glm::dvec4(omniCameraPosition, 1.0));

    cameraUp = glm::normalize(cameraUp);
    cameraFwd = glm::normalize(cameraFwd);

    double aspect = width / height;
    double verticalFov = 2.0 * glm::atan(1.0 / projMatrix[1][1]);
    double horizontalFov = 2.0 * glm::atan(glm::tan(verticalFov * 0.5) * aspect);

    return Cesium3DTilesSelection::ViewState::create(
        cameraPosition, cameraFwd, cameraUp, glm::dvec2(width, height), horizontalFov, verticalFov);
}

} // namespace cesium::omniverse
