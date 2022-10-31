#pragma once

#include "Event.h"

#include "cesium/omniverse/CesiumOmniverse.h"

#include <glm/glm.hpp>

namespace Cesium {
using OriginShiftHandler = event_handler<const glm::dmat4&, const glm::dmat4&>;

class CESIUM_OMNI_EXPORT_CLASS Georeference {
  public:
    static Georeference& instance();

    void setOrigin(const glm::dvec3& origin);

    event<const glm::dmat4&, const glm::dmat4&> originChangeEvent;
    glm::dmat4 absToRelWorld{1.0};
    glm::dmat4 relToAbsWorld{1.0};
};
} // namespace Cesium
