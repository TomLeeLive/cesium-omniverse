#pragma once

#include <pxr/base/gf/matrix4d.h>
#include <pxr/usd/usdUtils/stageCache.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

namespace cesium::omniverse {

inline pxr::UsdStageRefPtr getStage(long stageId) {
    return pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
}

inline glm::dmat4 gfToGlmMatrix(const pxr::GfMatrix4d& m) {
    // Row-major to column-major
    return glm::dmat4{
        m[0][0],
        m[1][0],
        m[2][0],
        m[3][0],
        m[0][1],
        m[1][1],
        m[2][1],
        m[3][1],
        m[0][2],
        m[1][2],
        m[2][2],
        m[3][2],
        m[0][3],
        m[1][3],
        m[2][3],
        m[3][3],
    };
}

inline pxr::GfMatrix4d glmToGfMatrix(const glm::dmat4& m) {
    // Column-major to row-major
    return pxr::GfMatrix4d{
        m[0][0],
        m[1][0],
        m[2][0],
        m[3][0],
        m[0][1],
        m[1][1],
        m[2][1],
        m[3][1],
        m[0][2],
        m[1][2],
        m[2][2],
        m[3][2],
        m[0][3],
        m[1][3],
        m[2][3],
        m[3][3],
    };
}

} // namespace cesium::omniverse
