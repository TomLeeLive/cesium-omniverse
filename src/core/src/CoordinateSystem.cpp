#include "cesium/omniverse/CoordinateSystem.h"

#include "cesium/omniverse/Util.h"

#include <CesiumGeometry/AxisTransforms.h>
#include <CesiumGeospatial/Cartographic.h>
#include <CesiumGeospatial/Transforms.h>
#include <glm/gtc/matrix_transform.hpp>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/tokens.h>

namespace cesium::omniverse {

namespace {
glm::dmat4 getAxisConversionMatrix(long stageId) {
    const auto& stage = getStage(stageId);
    const auto upAxis = pxr::UsdGeomGetStageUpAxis(stage);

    auto axisConversion = glm::dmat4(1.0);
    if (upAxis == pxr::UsdGeomTokens->y) {
        axisConversion = CesiumGeometry::AxisTransforms::Y_UP_TO_Z_UP;
    }

    return axisConversion;
}

glm::dmat4 getUnitConversionMatrix(long stageId) {
    const auto& stage = getStage(stageId);
    const auto metersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(stage);
    return glm::scale(glm::dmat4(1.0), glm::dvec3(metersPerUnit));
}

glm::dmat4 getEastNorthUpToFixedFrame(const CesiumGeospatial::Cartographic& cartographic) {
    const auto cartesian = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(cartographic);
    return CesiumGeospatial::Transforms::eastNorthUpToFixedFrame(cartesian);
}

} // namespace

void CoordinateSystem::setGeoreferenceOrigin(long stageId, const CesiumGeospatial::Cartographic& origin) {
    _localToGlobal =
        getEastNorthUpToFixedFrame(origin) * getAxisConversionMatrix(stageId) * getUnitConversionMatrix(stageId);

    _globalToLocal = glm::inverse(_localToGlobal);
}

void CoordinateSystem::setLocalOrigin(long stageId) {
    _localToGlobal = getAxisConversionMatrix(stageId) * getUnitConversionMatrix(stageId);
    _globalToLocal = glm::inverse(_localToGlobal);
}

const glm::dmat4& CoordinateSystem::getGlobalToLocal() const {
    return _globalToLocal;
}

const glm::dmat4& CoordinateSystem::getLocalToGlobal() const {
    return _localToGlobal;
}

} // namespace cesium::omniverse
