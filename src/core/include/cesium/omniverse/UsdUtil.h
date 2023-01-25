#pragma once

#include <carb/flatcache/StageWithHistory.h>
#include <glm/glm.hpp>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/common.h>
#include <usdrt/gf/matrix.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

namespace cesium::omniverse {

pxr::UsdStageRefPtr getUsdStage(long stageId);
usdrt::UsdStageRefPtr getUsdrtStage(long stageId);
carb::flatcache::StageInProgress getFabricStageInProgress(long stageId);
glm::dmat4 gfToGlmMatrix(const pxr::GfMatrix4d& matrix);
pxr::GfMatrix4d glmToGfMatrix(const glm::dmat4& matrix);
usdrt::GfMatrix4d glmToUsdrtGfMatrix(const glm::dmat4& matrix);
std::string printFabricStage(long stageId);
std::string printUsdrtStage(long stageId);
pxr::GfMatrix4d getUsdWorldTransform(pxr::SdfPath path);
pxr::TfToken getUsdUpAxis(long stageId);
double getUsdMetersPerUnit(long stageId);

} // namespace cesium::omniverse
