#pragma once

#include <carb/flatcache/StageWithHistory.h>
#include <glm/glm.hpp>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usd/common.h>
#include <usdrt/gf/matrix.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

namespace cesium::omniverse {

struct Decomposed {
    usdrt::GfVec3d position;
    usdrt::GfQuatf orientation;
    usdrt::GfVec3f scale;
};

pxr::UsdStageRefPtr getUsdStage(long stageId);
usdrt::UsdStageRefPtr getUsdrtStage(long stageId);
carb::flatcache::StageInProgress getFabricStageInProgress(long stageId);
glm::dmat4 usdToGlmMatrix(const pxr::GfMatrix4d& matrix);
glm::dmat4 usdrtToGlmMatrix(const usdrt::GfMatrix4d& matrix);
pxr::GfMatrix4d glmToUsdMatrix(const glm::dmat4& matrix);
usdrt::GfMatrix4d glmToUsdrtMatrix(const glm::dmat4& matrix);
Decomposed glmToUsdrtMatrixDecomposed(const glm::dmat4& matrix);
glm::dmat4 computeUsdWorldTransform(long stageId, const pxr::SdfPath& path);
pxr::TfToken getUsdUpAxis(long stageId);
double getUsdMetersPerUnit(long stageId);
pxr::SdfPath getChildOfRootPath(long stageId, const std::string& name);
pxr::SdfPath getChildOfRootPathUnique(long stageId, const std::string& name);

// TODO: are these not generic enough to belong in this file?
void updatePrimTransforms(long stageId, int tilesetId, const glm::dmat4& ecefToUsdTransform);
void updatePrimVisibility(long stageId, int tilesetId, bool visible);

std::string printFabricStage(long stageId);
std::string printUsdrtStage(long stageId);

} // namespace cesium::omniverse
