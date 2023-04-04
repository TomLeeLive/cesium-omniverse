#include "cesium/omniverse/FabricGeometry.h"

#include "cesium/omniverse/Context.h"
#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/FabricGeometryDefinition.h"
#include "cesium/omniverse/FabricMaterialDefinition.h"
#include "cesium/omniverse/GltfUtil.h"
#include "cesium/omniverse/Tokens.h"
#include "cesium/omniverse/UsdUtil.h"

#include <CesiumGltf/Model.h>
#include <carb/flatcache/FlatCacheUSD.h>

namespace cesium::omniverse {

namespace {

const auto TEXTURE_LOADING_COLOR = pxr::GfVec3f(0.0f, 1.0f, 0.0f);
const auto MATERIAL_LOADING_COLOR = pxr::GfVec3f(1.0f, 0.0f, 0.0f);
const auto DEFAULT_COLOR = pxr::GfVec3f(1.0f, 1.0f, 1.0f);

void deletePrimsFabric(const std::vector<pxr::SdfPath>& primsToDelete) {
    // Prims removed from Fabric need special handling for their removal to be reflected in the Hydra render index
    // This workaround may not be needed in future Kit versions, but is needed as of Kit 104.2
    auto sip = UsdUtil::getFabricStageInProgress();

    const carb::flatcache::Path changeTrackingPath("/TempChangeTracking");

    if (sip.getAttribute<uint64_t>(changeTrackingPath, FabricTokens::_deletedPrims) == nullptr) {
        return;
    }

    const auto deletedPrimsSize = sip.getArrayAttributeSize(changeTrackingPath, FabricTokens::_deletedPrims);
    sip.setArrayAttributeSize(changeTrackingPath, FabricTokens::_deletedPrims, deletedPrimsSize + primsToDelete.size());
    auto deletedPrimsFabric = sip.getArrayAttributeWr<uint64_t>(changeTrackingPath, FabricTokens::_deletedPrims);

    for (size_t i = 0; i < primsToDelete.size(); i++) {
        deletedPrimsFabric[deletedPrimsSize + i] = carb::flatcache::asInt(primsToDelete[i]).path;
    }
}

} // namespace

FabricGeometry::FabricGeometry(pxr::SdfPath path, const FabricGeometryDefinition& geometryDefinition)
    : _path(path) {

    auto sip = UsdUtil::getFabricStageInProgress();
    const auto pathFabric = carb::flatcache::Path(carb::flatcache::asInt(path));

    sip.createPrim(pathFabric);

    FabricAttributesBuilder attributes;
    attributes.addAttribute(FabricTypes::faceVertexCounts, FabricTokens::faceVertexCounts);
    attributes.addAttribute(FabricTypes::faceVertexIndices, FabricTokens::faceVertexIndices);
    attributes.addAttribute(FabricTypes::points, FabricTokens::points);
    attributes.addAttribute(FabricTypes::_localExtent, FabricTokens::_localExtent);
    attributes.addAttribute(FabricTypes::_worldExtent, FabricTokens::_worldExtent);
    attributes.addAttribute(FabricTypes::_worldVisibility, FabricTokens::_worldVisibility);
    attributes.addAttribute(FabricTypes::primvars, FabricTokens::primvars);
    attributes.addAttribute(FabricTypes::primvarInterpolations, FabricTokens::primvarInterpolations);
    attributes.addAttribute(FabricTypes::primvars_displayColor, FabricTokens::primvars_displayColor);
    attributes.addAttribute(FabricTypes::Mesh, FabricTokens::Mesh);
    attributes.addAttribute(FabricTypes::_cesium_tilesetId, FabricTokens::_cesium_tilesetId);
    attributes.addAttribute(FabricTypes::_cesium_tileId, FabricTokens::_cesium_tileId);
    attributes.addAttribute(FabricTypes::_cesium_localToEcefTransform, FabricTokens::_cesium_localToEcefTransform);
    attributes.addAttribute(FabricTypes::_worldPosition, FabricTokens::_worldPosition);
    attributes.addAttribute(FabricTypes::_worldOrientation, FabricTokens::_worldOrientation);
    attributes.addAttribute(FabricTypes::_worldScale, FabricTokens::_worldScale);
    attributes.addAttribute(FabricTypes::doubleSided, FabricTokens::doubleSided);
    attributes.addAttribute(FabricTypes::subdivisionScheme, FabricTokens::subdivisionScheme);

    if (geometryDefinition.getHasMaterial()) {
        attributes.addAttribute(FabricTypes::materialId, FabricTokens::materialId);
    }

    if (geometryDefinition.getHasSt()) {
        attributes.addAttribute(FabricTypes::primvars_st, FabricTokens::primvars_st);
    }

    if (geometryDefinition.getHasNormals()) {
        attributes.addAttribute(FabricTypes::primvars_normals, FabricTokens::primvars_normals);
    }

    attributes.createAttributes(pathFabric);

    // Some attributes need to be initialized right away
    auto subdivisionSchemeFabric =
        sip.getAttributeWr<carb::flatcache::Token>(pathFabric, FabricTokens::subdivisionScheme);
    *subdivisionSchemeFabric = FabricTokens::none;
}

FabricGeometry::~FabricGeometry() {
    // Only delete prims if there's still a stage to delete them from
    if (!UsdUtil::hasStage()) {
        return;
    }

    auto sip = UsdUtil::getFabricStageInProgress();

    sip.destroyPrim(carb::flatcache::asInt(_path));
    deletePrimsFabric({_path});
}

void FabricGeometry::initializeFromGltf(
    const FabricGeometryDefinition& geometryDefinition,
    int64_t tilesetId,
    int64_t tileId,
    const glm::dmat4& ecefToUsdTransform,
    const glm::dmat4& gltfToEcefTransform,
    const glm::dmat4& nodeTransform,
    const CesiumGltf::Model& model,
    const CesiumGltf::MeshPrimitive& primitive,
    bool smoothNormals,
    uint64_t imageryUvSetIndex) {
    auto sip = UsdUtil::getFabricStageInProgress();
    const auto pathFabric = carb::flatcache::Path(carb::flatcache::asInt(_path));

    const auto positions = GltfUtil::getPrimitivePositions(model, primitive);
    const auto indices = GltfUtil::getPrimitiveIndices(model, primitive, positions);
    const auto normals = GltfUtil::getPrimitiveNormals(model, primitive, positions, indices, smoothNormals);
    const auto st0 = GltfUtil::getPrimitiveUVs(model, primitive, 0);
    const auto imagerySt = GltfUtil::getImageryUVs(model, primitive, imageryUvSetIndex);
    const auto localExtent = GltfUtil::getPrimitiveExtent(model, primitive);
    const auto faceVertexCounts = GltfUtil::getPrimitiveFaceVertexCounts(indices);
    const auto doubleSided = GltfUtil::getDoubleSided(model, primitive);

    if (positions.empty() || indices.empty() || !localExtent.has_value()) {
        return;
    }

    const auto hasMaterial = geometryDefinition.getHasMaterial();
    const auto hasPrimitiveSt = !st0.empty();
    const auto hasImagerySt = !imagerySt.empty();
    const auto hasSt = hasPrimitiveSt || hasImagerySt;
    const auto hasNormals = !normals.empty();

    const auto localToEcefTransform = gltfToEcefTransform * nodeTransform;
    const auto localToUsdTransform = ecefToUsdTransform * localToEcefTransform;
    const auto [worldPosition, worldOrientation, worldScale] = UsdUtil::glmToUsdMatrixDecomposed(localToUsdTransform);
    const auto worldExtent = UsdUtil::computeWorldExtent(localExtent.value(), localToUsdTransform);

    size_t primvarsCount = 0;
    size_t primvarIndexSt = 0;
    size_t primvarIndexNormal = 0;

    const size_t primvarIndexDisplayColor = primvarsCount++;

    if (hasSt) {
        primvarIndexSt = primvarsCount++;
    }

    if (hasNormals) {
        primvarIndexNormal = primvarsCount++;
    }

    sip.setArrayAttributeSize(pathFabric, FabricTokens::faceVertexCounts, faceVertexCounts.size());
    sip.setArrayAttributeSize(pathFabric, FabricTokens::faceVertexIndices, indices.size());
    sip.setArrayAttributeSize(pathFabric, FabricTokens::points, positions.size());
    sip.setArrayAttributeSize(pathFabric, FabricTokens::primvars, primvarsCount);
    sip.setArrayAttributeSize(pathFabric, FabricTokens::primvarInterpolations, primvarsCount);
    sip.setArrayAttributeSize(pathFabric, FabricTokens::primvars_displayColor, 1);

    // clang-format off
    auto faceVertexCountsFabric = sip.getArrayAttributeWr<int>(pathFabric, FabricTokens::faceVertexCounts);
    auto faceVertexIndicesFabric = sip.getArrayAttributeWr<int>(pathFabric, FabricTokens::faceVertexIndices);
    auto pointsFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(pathFabric, FabricTokens::points);
    auto localExtentFabric = sip.getAttributeWr<pxr::GfRange3d>(pathFabric, FabricTokens::_localExtent);
    auto worldExtentFabric = sip.getAttributeWr<pxr::GfRange3d>(pathFabric, FabricTokens::_worldExtent);
    auto worldVisibilityFabric = sip.getAttributeWr<bool>(pathFabric, FabricTokens::_worldVisibility);
    auto primvarsFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(pathFabric, FabricTokens::primvars);
    auto primvarInterpolationsFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(pathFabric, FabricTokens::primvarInterpolations);
    auto displayColorFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(pathFabric, FabricTokens::primvars_displayColor);
    auto tilesetIdFabric = sip.getAttributeWr<int64_t>(pathFabric, FabricTokens::_cesium_tilesetId);
    auto tileIdFabric = sip.getAttributeWr<int64_t>(pathFabric, FabricTokens::_cesium_tileId);
    auto localToEcefTransformFabric = sip.getAttributeWr<pxr::GfMatrix4d>(pathFabric, FabricTokens::_cesium_localToEcefTransform);
    auto worldPositionFabric = sip.getAttributeWr<pxr::GfVec3d>(pathFabric, FabricTokens::_worldPosition);
    auto worldOrientationFabric = sip.getAttributeWr<pxr::GfQuatf>(pathFabric, FabricTokens::_worldOrientation);
    auto worldScaleFabric = sip.getAttributeWr<pxr::GfVec3f>(pathFabric, FabricTokens::_worldScale);
    auto doubleSidedFabric = sip.getAttributeWr<bool>(pathFabric, FabricTokens::doubleSided);
    auto subdivisionSchemeFabric = sip.getAttributeWr<carb::flatcache::Token>(pathFabric, FabricTokens::subdivisionScheme);
    // clang-format on

    std::copy(faceVertexCounts.begin(), faceVertexCounts.end(), faceVertexCountsFabric.begin());
    std::copy(indices.begin(), indices.end(), faceVertexIndicesFabric.begin());
    std::copy(positions.begin(), positions.end(), pointsFabric.begin());

    *worldVisibilityFabric = false;
    primvarsFabric[primvarIndexDisplayColor] = FabricTokens::primvars_displayColor;
    primvarInterpolationsFabric[primvarIndexDisplayColor] = FabricTokens::constant;
    *tilesetIdFabric = tilesetId;
    *tileIdFabric = tileId;
    *worldPositionFabric = worldPosition;
    *worldOrientationFabric = worldOrientation;
    *worldScaleFabric = worldScale;
    *localToEcefTransformFabric = UsdUtil::glmToUsdMatrix(localToEcefTransform);
    *doubleSidedFabric = doubleSided;
    *subdivisionSchemeFabric = FabricTokens::none;
    *localExtentFabric = localExtent.value();
    *worldExtentFabric = worldExtent;

    if (hasMaterial) {
        auto materialIdFabric = sip.getAttributeWr<uint64_t>(pathFabric, FabricTokens::materialId);
        *materialIdFabric = carb::flatcache::asInt(materialPaths[static_cast<size_t>(materialId)]).path;
        displayColorFabric[0] = MATERIAL_LOADING_COLOR;
    } else {
        displayColorFabric[0] = DEFAULT_COLOR;
    }

    if (hasSt) {
        const auto& st = hasImagerySt ? imagerySt : st0;

        sip.setArrayAttributeSize(pathFabric, FabricTokens::primvars_st, st.size());

        auto stFabric = sip.getArrayAttributeWr<pxr::GfVec2f>(pathFabric, FabricTokens::primvars_st);

        std::copy(st.begin(), st.end(), stFabric.begin());

        primvarsFabric[primvarIndexSt] = FabricTokens::primvars_st;
        primvarInterpolationsFabric[primvarIndexSt] = FabricTokens::vertex;
    }

    if (hasNormals) {
        sip.setArrayAttributeSize(pathFabric, FabricTokens::primvars_normals, normals.size());

        auto normalsFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(pathFabric, FabricTokens::primvars_normals);

        std::copy(normals.begin(), normals.end(), normalsFabric.begin());

        primvarsFabric[primvarIndexNormal] = FabricTokens::primvars_normals;
        primvarInterpolationsFabric[primvarIndexNormal] = FabricTokens::vertex;
    }
}

void FabricGeometry::assignMaterial(std::shared_ptr<FabricMaterial> material) {
    auto sip = UsdUtil::getFabricStageInProgress();
    auto materialIdFabric = sip.getAttributeWr<uint64_t>(_path, FabricTokens::materialId);
    *materialIdFabric = carb::flatcache::asInt(material.getPath()).path;
    auto displayColorFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(_path, FabricTokens::primvars_displayColor);
    displayColorFabric[0] = MATERIAL_LOADING_COLOR;
}

void FabricGeometry::setActive(bool active) {
    if (!active) {
        // TODO: should this also set arrays sizes back to zero to free memory?
        auto sip = UsdUtil::getFabricStageInProgress();
        auto worldVisibility = sip.getAttributeWr<bool>(carb::flatcache::asInt(_path), FabricTokens::_worldVisibility);
        *worldVisibility = false;

        // Make sure to unset the tileset and tile properties
    }
}

pxr::SdfPath FabricGeometry::getPath() const {
    return _path;
}

const FabricGeometryDefinition& FabricGeometry::getGeometryDefinition() const {
    return _geometryDefinition;
}

}; // namespace cesium::omniverse
