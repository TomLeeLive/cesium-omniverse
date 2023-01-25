#pragma once

#include <glm/glm.hpp>

namespace CesiumGeospatial {
class Cartographic;
}

namespace cesium::omniverse {

const glm::dmat4& computeUsdToEcef(long stageId, const CesiumGeospatial::Cartographic& origin);
const glm::dmat4& computeEcefToUsd(long stageId, const CesiumGeospatial::Cartographic& origin);

} // namespace cesium::omniverse
