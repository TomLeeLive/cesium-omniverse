#pragma once

#include <Cesium3DTilesSelection/ViewState.h>
#include <glm/glm.hpp>
#include <pxr/usd/sdf/path.h>

namespace CesiumGeospatial {
class Cartographic;
}

namespace cesium::omniverse::CoordinateSystemUtil {

glm::dmat4 computeUsdToEcefTransform(long stageId, const CesiumGeospatial::Cartographic& origin);
glm::dmat4 computeEcefToUsdTransform(long stageId, const CesiumGeospatial::Cartographic& origin);
glm::dmat4 computeEcefToUsdTransformForPrim(
    long stageId,
    const CesiumGeospatial::Cartographic& origin,
    const pxr::SdfPath& primPath);
Cesium3DTilesSelection::ViewState computeViewState(
    long stageId,
    const CesiumGeospatial::Cartographic& origin,
    const glm::dmat4& viewMatrix,
    const glm::dmat4& projMatrix,
    double width,
    double height);

} // namespace cesium::omniverse::CoordinateSystemUtil
