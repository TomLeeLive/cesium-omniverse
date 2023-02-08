#pragma once

#include "cesium/omniverse/UsdUtil.h"

#include <glm/gtx/matrix_decompose.hpp>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdUtils/stageCache.h>
#include <spdlog/fmt/fmt.h>

namespace cesium::omniverse::UsdUtil {

pxr::UsdStageRefPtr getUsdStage(long stageId) {
    return pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
}

usdrt::UsdStageRefPtr getUsdrtStage(long stageId) {
    return usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});
}

carb::flatcache::StageInProgress getFabricStageInProgressId(long stageId) {
    const auto stage = getUsdrtStage(stageId);
    return stage->GetStageInProgressId();
}

carb::flatcache::StageInProgress getFabricStageInProgress(long stageId) {
    const auto stage = getUsdrtStage(stageId);
    const auto stageInProgressId = stage->GetStageInProgressId();
    return carb::flatcache::StageInProgress(stageInProgressId);
}

glm::dmat4 usdToGlmMatrix(const pxr::GfMatrix4d& matrix) {
    // Row-major to column-major
    return glm::dmat4{
        matrix[0][0],
        matrix[1][0],
        matrix[2][0],
        matrix[3][0],
        matrix[0][1],
        matrix[1][1],
        matrix[2][1],
        matrix[3][1],
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2],
        matrix[0][3],
        matrix[1][3],
        matrix[2][3],
        matrix[3][3],
    };
}

glm::dmat4 usdrtToGlmMatrix(const usdrt::GfMatrix4d& matrix) {
    // Row-major to column-major
    return glm::dmat4{
        matrix[0][0],
        matrix[1][0],
        matrix[2][0],
        matrix[3][0],
        matrix[0][1],
        matrix[1][1],
        matrix[2][1],
        matrix[3][1],
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2],
        matrix[0][3],
        matrix[1][3],
        matrix[2][3],
        matrix[3][3],
    };
}

pxr::GfMatrix4d glmToUsdMatrix(const glm::dmat4& matrix) {
    // Column-major to row-major
    return pxr::GfMatrix4d{
        matrix[0][0],
        matrix[1][0],
        matrix[2][0],
        matrix[3][0],
        matrix[0][1],
        matrix[1][1],
        matrix[2][1],
        matrix[3][1],
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2],
        matrix[0][3],
        matrix[1][3],
        matrix[2][3],
        matrix[3][3],
    };
}

usdrt::GfMatrix4d glmToUsdrtMatrix(const glm::dmat4& matrix) {
    // Column-major to row-major
    return usdrt::GfMatrix4d{
        matrix[0][0],
        matrix[1][0],
        matrix[2][0],
        matrix[3][0],
        matrix[0][1],
        matrix[1][1],
        matrix[2][1],
        matrix[3][1],
        matrix[0][2],
        matrix[1][2],
        matrix[2][2],
        matrix[3][2],
        matrix[0][3],
        matrix[1][3],
        matrix[2][3],
        matrix[3][3],
    };
}

Decomposed glmToUsdrtMatrixDecomposed(const glm::dmat4& matrix) {
    glm::dvec3 scale;
    glm::dquat rotation;
    glm::dvec3 translation;
    glm::dvec3 skew;
    glm::dvec4 perspective;

    // TOOD: are the USD helpers faster? https://docs.omniverse.nvidia.com/prod_usd/prod_usd/python-snippets/transforms/get-world-transforms.html
    bool decomposable = glm::decompose(matrix, scale, rotation, translation, skew, perspective);
    assert(decomposable);

    glm::fquat rotationF32(rotation);
    glm::fvec3 scaleF32(scale);

    return {
        usdrt::GfVec3d(translation.x, translation.y, translation.z),
        usdrt::GfQuatf(rotationF32.w, usdrt::GfVec3f(rotationF32.x, rotationF32.y, rotationF32.z)),
        usdrt::GfVec3f(scaleF32.x, scaleF32.y, scaleF32.z),
    };
}

glm::dmat4 computeUsdWorldTransform(long stageId, const pxr::SdfPath& path) {
    const auto stage = getUsdStage(stageId);
    const auto prim = stage->GetPrimAtPath(path);
    assert(prim.IsValid());
    const auto xform = pxr::UsdGeomXformable(prim);
    const auto time = pxr::UsdTimeCode::Default();
    const auto transform = xform.ComputeLocalToWorldTransform(time);
    return usdToGlmMatrix(transform);
}

bool isPrimVisible(long stageId, const pxr::SdfPath& path) {
    // This is similar to isPrimVisible in kit-sdk/dev/include/omni/usd/UsdUtils.h
    const auto stage = getUsdStage(stageId);
    const auto prim = stage->GetPrimAtPath(path);
    assert(prim.IsValid());
    const auto imageable = pxr::UsdGeomImageable(prim);
    const auto time = pxr::UsdTimeCode::Default();
    const auto visibility = imageable.ComputeVisibility(time);
    return visibility != pxr::UsdGeomTokens->invisible;
}

pxr::TfToken getUsdUpAxis(long stageId) {
    const auto stage = getUsdStage(stageId);
    return pxr::UsdGeomGetStageUpAxis(stage);
}

double getUsdMetersPerUnit(long stageId) {
    const auto stage = getUsdStage(stageId);
    const auto metersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(stage);
    return metersPerUnit;
}

pxr::SdfPath getChildOfRootPath(long stageId, const std::string& name) {
    const auto stage = getUsdStage(stageId);
    return stage->GetPseudoRoot().GetPath().AppendChild(pxr::TfToken(name));
}

pxr::SdfPath getChildOfRootPathUnique(long stageId, const std::string& name) {
    const auto stage = getUsdStage(stageId);
    pxr::UsdPrim prim;
    pxr::SdfPath path;
    auto copy = 0;

    do {
        const auto copyName = copy > 0 ? fmt::format("{}_{}", name, copy) : name;
        path = getChildOfRootPath(stageId, name);
        prim = stage->GetPrimAtPath(path);
        copy++;
    } while (prim.IsValid());

    return path;
}

void setTilesetTransform(long stageId, int tilesetId, const glm::dmat4& ecefToUsdTransform) {
    auto sip = getFabricStageInProgress(stageId);

    // These should match the type/name in convertPrimitiveToUsd in GltfToUSD
    carb::flatcache::Type tilesetIdType(carb::flatcache::BaseDataType::eInt, 1, 0);
    carb::flatcache::Token tilesetIdToken("_tilesetId");
    carb::flatcache::Token localMatrixToken("_localMatrix");
    carb::flatcache::Token worldPosToken("_worldPosition");
    carb::flatcache::Token worldOrToken("_worldOrientation");
    carb::flatcache::Token worldScaleToken("_worldScale");

    const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(tilesetIdType, tilesetIdToken)});

    for (auto bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto tilesetIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tilesetIdToken);
        const auto localMatrixArray = sip.getAttributeArrayRd<usdrt::GfMatrix4d>(buckets, bucketId, localMatrixToken);

        // Technically _worldPosition type is Double[3] it's fine to reinterpret as GfVec3d
        // TODO: is it a problem that rotation and scale are 32-bit?
        const auto worldPositionArray = sip.getAttributeArrayWr<usdrt::GfVec3d>(buckets, bucketId, worldPosToken);
        const auto worldOrientationArray = sip.getAttributeArrayWr<usdrt::GfQuatf>(buckets, bucketId, worldOrToken);
        const auto worldScaleArray = sip.getAttributeArrayWr<usdrt::GfVec3f>(buckets, bucketId, worldScaleToken);

        for (auto i = 0; i < tilesetIdArray.size(); i++) {
            if (tilesetIdArray[i] == tilesetId) {
                auto localToEcefTransform = usdrtToGlmMatrix(localMatrixArray[i]);
                auto localToUsdTransform = ecefToUsdTransform * localToEcefTransform;
                auto [position, orientation, scale] = glmToUsdrtMatrixDecomposed(localToUsdTransform);
                worldPositionArray[i] = position;
                worldOrientationArray[i] = orientation;
                worldScaleArray[i] = scale;
            }
        }
    }
}

void setTilesetVisibility(long stageId, int tilesetId, bool visible) {
    auto sip = getFabricStageInProgress(stageId);

    // These should match the type/name in convertPrimitiveToUsd in GltfToUSD
    carb::flatcache::Type tilesetIdType(carb::flatcache::BaseDataType::eInt, 1, 0);
    carb::flatcache::Token tilesetIdToken("_tilesetId");
    carb::flatcache::Token visibilityToken("visibility");

    const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(tilesetIdType, tilesetIdToken)});

    for (auto bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto tilesetIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tilesetIdToken);
        const auto visibilityArray = sip.getAttributeArrayWr<bool>(buckets, bucketId, visibilityToken);

        for (auto i = 0; i < tilesetIdArray.size(); i++) {
            if (tilesetIdArray[i] == tilesetId) {
                visibilityArray[i] = visible;
            }
        }
    }
}

void setTileVisibility(long stageId, int tilesetId, int tileId, bool visible) {
    auto sip = getFabricStageInProgress(stageId);

    // These should match the type/name in convertPrimitiveToUsd in GltfToUSD
    carb::flatcache::Type tilesetIdType(carb::flatcache::BaseDataType::eInt, 1, 0);

    carb::flatcache::Token tilesetIdToken("_tilesetId");
    carb::flatcache::Token tileIdToken("_tileId");
    carb::flatcache::Token visibilityToken("visibility");

    const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(tilesetIdType, tilesetIdToken)});

    for (auto bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto tilesetIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tilesetIdToken);
        const auto tileIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tileIdToken);
        const auto visibilityArray = sip.getAttributeArrayWr<bool>(buckets, bucketId, visibilityToken);

        for (auto i = 0; i < tilesetIdArray.size(); i++) {
            if (tilesetIdArray[i] == tilesetId && tileIdArray[i] == tileId) {
                visibilityArray[i] = visible;
            }
        }
    }
}

void destroyTile(long stageId, int tilesetId, int tileId) {
    auto sip = getFabricStageInProgress(stageId);

    // These should match the type/name in convertPrimitiveToUsd in GltfToUSD
    carb::flatcache::Type tilesetIdType(carb::flatcache::BaseDataType::eInt, 1, 0);

    carb::flatcache::Token tilesetIdToken("_tilesetId");
    carb::flatcache::Token tileIdToken("_tileId");

    const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(tilesetIdType, tilesetIdToken)});

    // Reuse the same scratch vector and don't resize its capacity
    thread_local std::vector<carb::flatcache::Path> primsToDelete;
    primsToDelete.clear();

    for (auto bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto tilesetIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tilesetIdToken);
        const auto tileIdArray = sip.getAttributeArrayRd<int>(buckets, bucketId, tileIdToken);
        const auto primPaths = sip.getPathArray(buckets, bucketId);

        for (auto i = 0; i < tilesetIdArray.size(); i++) {
            if (tilesetIdArray[i] == tilesetId && tileIdArray[i] == tileId) {
                primsToDelete.push_back(primPaths[i]);
            }
        }
    }

    for (auto i = 0; i < primsToDelete.size(); i++) {
        sip.destroyPrim(primsToDelete[i]);
    }

    // Prims removed from Fabric need special handling for their removal to be reflected in the Hydra render index
    // This workaround may not be needed in future Kit versions
    carb::flatcache::Path changeTrackingPath("/TempChangeTracking");
    carb::flatcache::Token deletedPrimsAttribute("_deletedPrims");

    const auto deletedPrimsSize = sip.getArrayAttributeSize(changeTrackingPath, deletedPrimsAttribute);
    sip.setArrayAttributeSize(changeTrackingPath, deletedPrimsAttribute, deletedPrimsSize + primsToDelete.size());
    auto deletedPrimsFabric = sip.getArrayAttributeWr<carb::flatcache::Path>(changeTrackingPath, deletedPrimsAttribute);

    for (auto i = 0; i < primsToDelete.size(); i++) {
        deletedPrimsFabric[deletedPrimsSize + i] = primsToDelete[i];
    }
}

namespace {

class TokenWrapper {
  private:
    carb::flatcache::TokenC tokenC;

  public:
    friend std::ostream& operator<<(std::ostream& os, const TokenWrapper& tokenWrapper);
};

std::ostream& operator<<(std::ostream& os, const TokenWrapper& tokenWrapper) {
    os << carb::flatcache::Token(tokenWrapper.tokenC).getString();
    return os;
}

class BoolWrapper {
  private:
    bool value;

  public:
    friend std::ostream& operator<<(std::ostream& os, const BoolWrapper& boolWrapper);
};

std::ostream& operator<<(std::ostream& os, const BoolWrapper& boolWrapper) {
    os << boolWrapper.value ? "true" : "false";
    return os;
}

// TODO: the asset always refers to `\0` empty string and causes this class to be all zeros, which gets printed as "" in << operator
class AssetWrapper {
  private:
    uint64_t padding[8];

  public:
    friend std::ostream& operator<<(std::ostream& os, const AssetWrapper& assetWrapper);
};

std::ostream& operator<<(std::ostream& os, const AssetWrapper& assetWrapper) {
    const auto& padding = assetWrapper.padding;
    os << "bytes: " << padding[0] << " " << padding[1] << " " << padding[2] << " " << padding[3] << " " << padding[4] << " "
       << padding[5] << " " << padding[6] << " " << padding[7];
    return os;
}

template <typename T> std::string toString(const T& value) {
    return (std::stringstream() << value).str();
}

template <typename T>
std::string printAttributeValuePtr(const T* values, uint64_t elementCount, uint64_t componentCount) {
    std::stringstream stream;

    if (elementCount > 1) {
        stream << "[";
    }

    for (uint64_t i = 0; i < elementCount; i++) {
        if (componentCount > 1) {
            stream << "[";
        }

        for (uint64_t j = 0; j < componentCount; j++) {
            stream << values[i * componentCount + j];
            if (j < componentCount - 1) {
                stream << ",";
            }
        }

        if (componentCount > 1) {
            stream << "]";
        }

        if (elementCount > 1 && i < elementCount - 1) {
            stream << ",";
        }
    }

    if (elementCount > 1) {
        stream << "]";
    }

    return stream.str();
}

template <typename BaseType, uint64_t ComponentCount>
std::string printAttributeValueUsdrt(const usdrt::UsdAttribute& attribute) {
    using ElementType = std::array<BaseType, ComponentCount>;
    usdrt::VtArray<ElementType> values;
    attribute.Get<ElementType>(&values);
    const uint64_t elementCount = values.size();

    if (elementCount == 0) {
        return "";
    }

    return printAttributeValuePtr<BaseType>(values.data()->data(), elementCount, ComponentCount);
}

template <typename BaseType, uint64_t ComponentCount>
std::string printAttributeValueUsd(const pxr::UsdAttribute& attribute) {
    using ElementType = std::array<BaseType, ComponentCount>;
    usdrt::VtArray<ElementType> values;
    attribute.Get<ElementType>(&values);
    const uint64_t elementCount = values.size();

    if (elementCount == 0) {
        return "";
    }

    return printAttributeValuePtr<BaseType>(values.data()->data(), elementCount, ComponentCount);
}

template <bool IsArray, typename BaseType, uint64_t ComponentCount>
std::string printAttributeValueFabric(
    carb::flatcache::StageInProgress& stageInProgress,
    const carb::flatcache::Path& primPath,
    const carb::flatcache::Token& attributeName) {
    using ElementType = std::array<BaseType, ComponentCount>;

    if constexpr (IsArray) {
        const auto values = stageInProgress.getArrayAttributeRd<ElementType>(primPath, attributeName);
        const uint64_t elementCount = values.size();

        if (elementCount == 0) {
            return "";
        }

        return printAttributeValuePtr<BaseType>(values.front().data(), elementCount, ComponentCount);
    } else {
        const auto value = stageInProgress.getAttributeRd<ElementType>(primPath, attributeName);

        if (value == nullptr) {
            return "";
        }

        return printAttributeValuePtr<BaseType>(value->data(), 1, ComponentCount);
    }
}

std::string getTypeName(const std::string& typeName) {
    if (typeName == "") {
        return "[None]";
    }

    return typeName;
}

std::string printUsdrtPrim(const usdrt::UsdPrim& prim, const bool printChildren) {
    std::stringstream stream;

    const auto& name = prim.GetName().GetString();
    const auto& primPath = prim.GetPrimPath().GetString();
    const auto& typeName = prim.GetTypeName().GetString();
    const auto& attributes = prim.GetAttributes();

    const uint64_t tabSize = 2;
    const uint64_t depth = std::count(primPath.begin(), primPath.end(), '/');
    const std::string primSpaces(depth * tabSize, ' ');
    const std::string primInfoSpaces((depth + 1) * tabSize, ' ');
    const std::string attributeSpaces((depth + 2) * tabSize, ' ');
    const std::string attributeInfoSpaces((depth + 3) * tabSize, ' ');

    stream << fmt::format("{}Prim: {}\n", primSpaces, primPath);
    stream << fmt::format("{}Name: {}\n", primInfoSpaces, name);
    stream << fmt::format("{}Type: {}\n", primInfoSpaces, getTypeName(typeName));

    if (!attributes.empty()) {
        stream << fmt::format("{}Attributes:\n", primInfoSpaces);
    }

    for (const auto& attribute : attributes) {
        const auto& attributeName = attribute.GetName().GetString();
        const auto& attributeTypeName = attribute.GetTypeName().GetAsString();

        stream << fmt::format("{}Attribute: {}\n", attributeSpaces, attributeName);
        stream << fmt::format("{}Type: {}\n", attributeInfoSpaces, attributeTypeName);

        if (attribute.HasValue()) {
            const auto attributeType = carb::flatcache::Type(attribute.GetTypeName().GetAsTypeC());
            const auto baseType = attributeType.baseType;
            const auto componentCount = attributeType.componentCount;

            std::string attributeValue;
            bool typeNotSupported = false;

            // These switch statements should cover all the attribute types we expect to see used by USDRT
            // prims. See SdfValueTypeNames for the full list.
            switch (baseType) {
                case carb::flatcache::BaseDataType::eToken: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<TokenWrapper, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eBool: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<BoolWrapper, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eUChar: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<uint8_t, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eInt: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<int32_t, 1>(attribute);
                            break;
                        }
                        case 2: {
                            attributeValue = printAttributeValueUsdrt<int32_t, 2>(attribute);
                            break;
                        }
                        case 3: {
                            attributeValue = printAttributeValueUsdrt<int32_t, 3>(attribute);
                            break;
                        }
                        case 4: {
                            attributeValue = printAttributeValueUsdrt<int32_t, 4>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eUInt: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<uint32_t, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eInt64: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<int64_t, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eUInt64: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<uint64_t, 1>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eFloat: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<float, 1>(attribute);
                            break;
                        }
                        case 2: {
                            attributeValue = printAttributeValueUsdrt<float, 2>(attribute);
                            break;
                        }
                        case 3: {
                            attributeValue = printAttributeValueUsdrt<float, 3>(attribute);
                            break;
                        }
                        case 4: {
                            attributeValue = printAttributeValueUsdrt<float, 4>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                case carb::flatcache::BaseDataType::eDouble: {
                    switch (componentCount) {
                        case 1: {
                            attributeValue = printAttributeValueUsdrt<double, 1>(attribute);
                            break;
                        }
                        case 2: {
                            attributeValue = printAttributeValueUsdrt<double, 2>(attribute);
                            break;
                        }
                        case 3: {
                            attributeValue = printAttributeValueUsdrt<double, 3>(attribute);
                            break;
                        }
                        case 4: {
                            attributeValue = printAttributeValueUsdrt<double, 4>(attribute);
                            break;
                        }
                        case 6: {
                            attributeValue = printAttributeValueUsdrt<double, 6>(attribute);
                            break;
                        }
                        case 9: {
                            attributeValue = printAttributeValueUsdrt<double, 9>(attribute);
                            break;
                        }
                        case 16: {
                            attributeValue = printAttributeValueUsdrt<double, 16>(attribute);
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                    break;
                }
                default: {
                    typeNotSupported = true;
                    break;
                }
            }

            if (typeNotSupported) {
                attributeValue = "[Type not supported]";
            } else if (attributeValue == "") {
                attributeValue = "[Value not in Fabric]";
            }

            stream << fmt::format("{}Value: {}\n", attributeInfoSpaces, attributeValue);
        }
    }

    if (printChildren) {
        for (const auto& child : prim.GetChildren()) {
            stream << printUsdrtPrim(child, printChildren);
        }
    }

    return stream.str();
}

inline bool hasFabricGpuData(long stageId, const carb::flatcache::Path& path, const carb::flatcache::Token& attr) {
    auto sip = getFabricStageInProgress(stageId);
    auto validBits = sip.getAttributeValidBits(path, attr);
    return uint32_t(validBits & carb::flatcache::ValidMirrors::eCudaGPU) != 0;
}

} // namespace

std::string printUsdrtStage(long stageId) {
    const auto stage = getUsdrtStage(stageId);

    // Notes aboute Traverse() and PrimRange from the USDRT docs:
    //
    // * PrimRanges that access a prim for the first time will cause that prim and its attributes to be loaded into Fabric
    // * Prims that are defined only in Fabric will not appear in PrimRange results
    // * Attributes that only have fallback values are currently not populated into Fabric.

    std::stringstream stream;
    for (const auto& prim : stage->Traverse()) {
        stream << printUsdrtPrim(prim, false);
    }
    return stream.str();

    // Alternative:

    // const auto root = stage->GetPseudoRoot();
    // return printUsdrtPrim(root, true);
}

std::string printFabricStage(long stageId) {
    auto sip = getFabricStageInProgress(stageId);

    // For extra debugging
    sip.printBucketNames();

    std::stringstream stream;

    const auto& buckets = sip.findPrims({});

    for (uint64_t bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto& attributes = sip.getAttributeNamesAndTypes(buckets, bucketId);
        const auto& primPaths = sip.getPathArray(buckets, bucketId);

        for (const auto& primPath : primPaths) {
            const std::string primPathString = primPath.getText();
            const uint64_t primPathUint64 = carb::flatcache::PathC(primPath).path;
            const uint64_t tabSize = 2;
            const uint64_t depth = std::count(primPathString.begin(), primPathString.end(), '/');
            const std::string primSpaces(depth * tabSize, ' ');
            const std::string primInfoSpaces((depth + 1) * tabSize, ' ');
            const std::string attributeSpaces((depth + 2) * tabSize, ' ');
            const std::string attributeInfoSpaces((depth + 3) * tabSize, ' ');

            stream << fmt::format("{}Prim: {}\n", primSpaces, primPathString);
            stream << fmt::format("{}Prim Path Uint64: {}\n", primInfoSpaces, primPathUint64);
            stream << fmt::format("{}Attributes:\n", primInfoSpaces);

            for (const auto& attribute : attributes) {
                const auto attributeType = attribute.type;
                const auto baseType = attributeType.baseType;
                const auto componentCount = attributeType.componentCount;
                const auto role = attributeType.role;
                const auto name = attribute.name;
                const auto arrayDepth = attributeType.arrayDepth;
                const auto attributeNameString = name.getString();
                const auto attributeTypeString = attributeType.getTypeName();
                const auto roleString = toString(role);

                std::string attributeValue;
                bool typeNotSupported = false;

                // These switch statements should cover all the attribute types we expect to see used by USDRT
                // prims. See SdfValueTypeNames for the full list. Fabric allows for many more attribute types
                // but we don't really care at the moment since we're always interfacing with Fabric via USDRT.
                if (arrayDepth == 0) {
                    switch (baseType) {
                        case carb::flatcache::BaseDataType::eAsset: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<false, AssetWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eToken: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<false, TokenWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eBool: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<false, BoolWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUChar: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, uint8_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eInt: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, int32_t, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<false, int32_t, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<false, int32_t, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<false, int32_t, 4>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUInt: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, uint32_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eInt64: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, int64_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUInt64: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, uint64_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eFloat: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, float, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<false, float, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<false, float, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<false, float, 4>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eDouble: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<false, double, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<false, double, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<false, double, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<false, double, 4>(sip, primPath, name);
                                    break;
                                }
                                case 6: {
                                    attributeValue = printAttributeValueFabric<false, double, 6>(sip, primPath, name);
                                    break;
                                }
                                case 9: {
                                    attributeValue = printAttributeValueFabric<false, double, 9>(sip, primPath, name);
                                    break;
                                }
                                case 16: {
                                    attributeValue = printAttributeValueFabric<false, double, 16>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                } else if (arrayDepth == 1) {
                    switch (baseType) {
                        case carb::flatcache::BaseDataType::eAsset: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<true, AssetWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eToken: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<true, TokenWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eBool: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue =
                                        printAttributeValueFabric<true, BoolWrapper, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUChar: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, uint8_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eInt: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, int32_t, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<true, int32_t, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<true, int32_t, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<true, int32_t, 4>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUInt: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, uint32_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eInt64: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, int64_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eUInt64: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, uint64_t, 1>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eFloat: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, float, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<true, float, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<true, float, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<true, float, 4>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        case carb::flatcache::BaseDataType::eDouble: {
                            switch (componentCount) {
                                case 1: {
                                    attributeValue = printAttributeValueFabric<true, double, 1>(sip, primPath, name);
                                    break;
                                }
                                case 2: {
                                    attributeValue = printAttributeValueFabric<true, double, 2>(sip, primPath, name);
                                    break;
                                }
                                case 3: {
                                    attributeValue = printAttributeValueFabric<true, double, 3>(sip, primPath, name);
                                    break;
                                }
                                case 4: {
                                    attributeValue = printAttributeValueFabric<true, double, 4>(sip, primPath, name);
                                    break;
                                }
                                case 6: {
                                    attributeValue = printAttributeValueFabric<true, double, 6>(sip, primPath, name);
                                    break;
                                }
                                case 9: {
                                    attributeValue = printAttributeValueFabric<true, double, 9>(sip, primPath, name);
                                    break;
                                }
                                case 16: {
                                    attributeValue = printAttributeValueFabric<true, double, 16>(sip, primPath, name);
                                    break;
                                }
                                default: {
                                    typeNotSupported = true;
                                    break;
                                }
                            }
                            break;
                        }
                        default: {
                            typeNotSupported = true;
                            break;
                        }
                    }
                }

                if (typeNotSupported) {
                    attributeValue = "[Type not supported]";
                } else if (attributeValue == "") {
                    attributeValue = "[Value not in Fabric]";
                }

                stream << fmt::format("{}Attribute: {}\n", attributeSpaces, attributeNameString);
                stream << fmt::format("{}Type: {}\n", attributeInfoSpaces, attributeTypeString);
                stream << fmt::format("{}GPU: {}\n", attributeInfoSpaces, hasFabricGpuData(stageId, primPath, name));

                if (baseType != carb::flatcache::BaseDataType::eTag) {
                    stream << fmt::format("{}Value: {}\n", attributeInfoSpaces, attributeValue);
                }
            }
        }
    }

    return stream.str();
}

} // namespace cesium::omniverse::UsdUtil
