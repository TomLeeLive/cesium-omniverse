#pragma once

#include <carb/flatcache/StageWithHistory.h>
#include <glm/glm.hpp>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usd/common.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

namespace cesium::omniverse {

pxr::UsdStageRefPtr getUsdStage(long stageId);
usdrt::UsdStageRefPtr getUsdrtStage(long stageId);
carb::flatcache::StageInProgress getFabricStageInProgress(long stageId);
glm::dmat4 gfToGlmMatrix(const pxr::GfMatrix4d& matrix);
pxr::GfMatrix4d glmToGfMatrix(const glm::dmat4& matrix);
std::string printFabricStage(long stageId);
std::string printUsdrtStage(long stageId);

} // namespace cesium::omniverse
