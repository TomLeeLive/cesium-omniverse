#pragma once

#include <glm/glm.hpp>

namespace CesiumGeospatial {
class Cartographic;
}

namespace cesium::omniverse {

class CoordinateSystem {
  public:
    void setGeoreferenceOrigin(long stageId, const CesiumGeospatial::Cartographic& origin);
    void setLocalOrigin(long stageId);

    const glm::dmat4& getGlobalToLocal() const;
    const glm::dmat4& getLocalToGlobal() const;

  private:
    glm::dmat4 _globalToLocal{1.0};
    glm::dmat4 _localToGlobal{1.0};
};
} // namespace cesium::omniverse
