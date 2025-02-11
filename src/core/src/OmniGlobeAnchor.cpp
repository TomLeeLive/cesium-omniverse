#include "cesium/omniverse/OmniGlobeAnchor.h"

#include "cesium/omniverse/GeospatialUtil.h"
#include "cesium/omniverse/UsdUtil.h"

#include <glm/ext/matrix_relational.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <utility>

namespace cesium::omniverse {

const pxr::GfMatrix4d& OmniGlobeAnchor::getCachedTransformation() const {
    return _valueCache.transformation;
}

const pxr::GfVec3d& OmniGlobeAnchor::getCachedGeographicCoordinate() const {
    return _valueCache.geographicCoordinate;
}

const pxr::GfVec3d& OmniGlobeAnchor::getCachedEcefPosition() const {
    return _valueCache.ecefPosition;
}

const pxr::GfVec3d& OmniGlobeAnchor::getCachedEcefRotation() const {
    return _valueCache.ecefRotation;
}

const pxr::GfVec3d& OmniGlobeAnchor::getCachedEcefScale() const {
    return _valueCache.ecefScale;
}

void OmniGlobeAnchor::updateCachedValues() {
    auto globeAnchorAPI = UsdUtil::getCesiumGlobeAnchor(_anchorPrimPath);

    pxr::GfVec3d newGeographicCoordinate;
    globeAnchorAPI.GetGeographicCoordinateAttr().Get(&newGeographicCoordinate);
    _valueCache.geographicCoordinate = newGeographicCoordinate;

    pxr::GfVec3d newEcefPosition;
    globeAnchorAPI.GetPositionAttr().Get(&newEcefPosition);
    _valueCache.ecefPosition = newEcefPosition;

    pxr::GfVec3d newEcefRotation;
    globeAnchorAPI.GetRotationAttr().Get(&newEcefRotation);
    _valueCache.ecefRotation = newEcefRotation;

    pxr::GfVec3d newEcefScale;
    globeAnchorAPI.GetScaleAttr().Get(&newEcefScale);
    _valueCache.ecefScale = newEcefScale;

    auto maybeNewTransform = UsdUtil::getCesiumTransformOpValueForPathIfExists(_anchorPrimPath);
    if (maybeNewTransform.has_value()) {
        _valueCache.transformation = maybeNewTransform.value();
    }
}

OmniGlobeAnchor::OmniGlobeAnchor(pxr::SdfPath anchorPrimPath, const glm::dmat4& anchorToFixed)
    : _anchorPrimPath{std::move(anchorPrimPath)} {
    _anchor = std::make_shared<CesiumGeospatial::GlobeAnchor>(anchorToFixed);
}

const glm::dmat4& OmniGlobeAnchor::getAnchorToFixedTransform() const {
    return _anchor->getAnchorToFixedTransform();
}

const glm::dmat4 OmniGlobeAnchor::getAnchorToLocalTransform(const CesiumGeospatial::Cartographic& origin) {
    return _anchor->getAnchorToLocalTransform(GeospatialUtil::getCoordinateSystem(origin));
}

std::optional<CesiumGeospatial::Cartographic> OmniGlobeAnchor::getCartographicPosition() {
    glm::dvec3 fs{};
    glm::dquat fr{};
    glm::dvec3 ft{};
    glm::dvec3 fskew{};
    glm::dvec4 fperspective{};

    [[maybe_unused]] auto fixedTransform = glm::decompose(getAnchorToFixedTransform(), fs, fr, ft, fskew, fperspective);
    assert(fixedTransform);

    return CesiumGeospatial::Ellipsoid::WGS84.cartesianToCartographic(ft);
}

const pxr::SdfPath& OmniGlobeAnchor::getPrimPath() const {
    return _anchorPrimPath;
}

void OmniGlobeAnchor::updateByFixedTransform(
    const glm::dvec3& ecefPositionVec,
    const glm::dvec3& ecefRotationRadVec,
    const glm::dvec3& ecefScaleVec,
    bool shouldReorient) {
    auto translation = glm::translate(glm::dmat4(1.0), ecefPositionVec);
    auto rotation = glm::eulerAngleXYZ<double>(ecefRotationRadVec.x, ecefRotationRadVec.y, ecefRotationRadVec.z);
    auto scale = glm::scale(glm::dmat4(1.0), ecefScaleVec);
    auto newAnchorToFixed = translation * rotation * scale;

    _anchor->setAnchorToFixedTransform(newAnchorToFixed, shouldReorient);
}

void OmniGlobeAnchor::updateByGeographicCoordinates(CesiumGeospatial::Cartographic& cartographic, bool shouldReorient) {
    auto newEcefPositionVec = CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(cartographic);

    auto ecefRotationDegVec = UsdUtil::usdToGlmVector(_valueCache.ecefRotation);
    auto ecefScaleVec = UsdUtil::usdToGlmVector(_valueCache.ecefScale);

    updateByFixedTransform(newEcefPositionVec, glm::radians(ecefRotationDegVec), ecefScaleVec, shouldReorient);
}

void OmniGlobeAnchor::updateByUsdTransform(const CesiumGeospatial::Cartographic& origin, bool shouldReorient) {
    const auto newAnchorToFixed = UsdUtil::computeUsdLocalToEcefTransformForPrim(origin, _anchorPrimPath);

    _anchor->setAnchorToFixedTransform(newAnchorToFixed, shouldReorient);
}

} // namespace cesium::omniverse
