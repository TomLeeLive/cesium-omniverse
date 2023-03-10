#include "cesium/omniverse/FabricUtil.h"

#include "cesium/omniverse/FabricAsset.h"
#include "cesium/omniverse/FabricAttributesBuilder.h"
#include "cesium/omniverse/Tokens.h"
#include "cesium/omniverse/UsdUtil.h"

#include <carb/flatcache/FlatCacheUSD.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/quatf.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <spdlog/fmt/fmt.h>

#include <sstream>

namespace cesium::omniverse::FabricUtil {

namespace {

const char* const NO_DATA_STRING = "[No Data]";
const char* const TYPE_NOT_SUPPORTED_STRING = "[Type Not Supported]";

// Wraps the token type so that we can define a custom stream insertion operator
class TokenWrapper {
  private:
    carb::flatcache::Token token;

  public:
    friend std::ostream& operator<<(std::ostream& os, const TokenWrapper& tokenWrapper);
};

std::ostream& operator<<(std::ostream& os, const TokenWrapper& tokenWrapper) {
    os << tokenWrapper.token.getString();
    return os;
}

// Wraps a boolean so that we print "true" and "false" instead of 0 and 1
class BoolWrapper {
  private:
    bool value;

  public:
    friend std::ostream& operator<<(std::ostream& os, const BoolWrapper& boolWrapper);
};

std::ostream& operator<<(std::ostream& os, const BoolWrapper& boolWrapper) {
    os << (boolWrapper.value ? "true" : "false");
    return os;
}

class AssetWrapper {
  private:
    FabricAsset asset;

  public:
    friend std::ostream& operator<<(std::ostream& os, const AssetWrapper& assetWrapper);
};

std::ostream& operator<<(std::ostream& os, const AssetWrapper& assetWrapper) {
    if (assetWrapper.asset.isEmpty()) {
        os << NO_DATA_STRING;
        return os;
    }

    assert(assetWrapper.asset.isPaddingEmpty());

    os << "Asset Path: " << assetWrapper.asset.getAssetPath()
       << ", Resolved Path: " << assetWrapper.asset.getResolvedPath();
    return os;
}

template <typename T>
std::string printAttributeValue(const T* values, uint64_t elementCount, uint64_t componentCount, bool isArray) {
    std::stringstream stream;

    if (isArray) {
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

    if (isArray) {
        stream << "]";
    }

    return stream.str();
}

template <bool IsArray, typename BaseType, uint64_t ComponentCount>
std::string printAttributeValue(const carb::flatcache::Path& primPath, const carb::flatcache::Token& attributeName) {
    using ElementType = std::array<BaseType, ComponentCount>;

    auto stageInProgress = UsdUtil::getFabricStageInProgress();

    if constexpr (IsArray) {
        const auto values = stageInProgress.getArrayAttributeRd<ElementType>(primPath, attributeName);
        const auto elementCount = values.size();

        if (elementCount == 0) {
            return NO_DATA_STRING;
        }

        return printAttributeValue<BaseType>(values.front().data(), elementCount, ComponentCount, true);
    } else {
        const auto value = stageInProgress.getAttributeRd<ElementType>(primPath, attributeName);

        if (value == nullptr) {
            return NO_DATA_STRING;
        }

        return printAttributeValue<BaseType>(value->data(), 1, ComponentCount, false);
    }
}

std::string
printAttributeValue(const carb::flatcache::Path& primPath, const carb::flatcache::AttrNameAndType& attribute) {
    auto stageInProgress = UsdUtil::getFabricStageInProgress();

    const auto attributeType = attribute.type;
    const auto baseType = attributeType.baseType;
    const auto componentCount = attributeType.componentCount;
    const auto name = attribute.name;
    const auto arrayDepth = attributeType.arrayDepth;

    // This switch statement should cover most of the attribute types we expect to see on the stage.
    // This includes the USD types in SdfValueTypeNames and Fabric types like assets and tokens.
    // We can add more as needed.
    if (arrayDepth == 0) {
        switch (baseType) {
            case carb::flatcache::BaseDataType::eAsset: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, AssetWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eToken: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, TokenWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eBool: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, BoolWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUChar: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, uint8_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eInt: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, int32_t, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<false, int32_t, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<false, int32_t, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<false, int32_t, 4>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUInt: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, uint32_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eInt64: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, int64_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUInt64: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, uint64_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eFloat: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, float, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<false, float, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<false, float, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<false, float, 4>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eDouble: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<false, double, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<false, double, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<false, double, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<false, double, 4>(primPath, name);
                    }
                    case 6: {
                        return printAttributeValue<false, double, 6>(primPath, name);
                    }
                    case 9: {
                        return printAttributeValue<false, double, 9>(primPath, name);
                    }
                    case 16: {
                        return printAttributeValue<false, double, 16>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    } else if (arrayDepth == 1) {
        switch (baseType) {
            case carb::flatcache::BaseDataType::eAsset: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, AssetWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eToken: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, TokenWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eBool: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, BoolWrapper, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUChar: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, uint8_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eInt: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, int32_t, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<true, int32_t, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<true, int32_t, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<true, int32_t, 4>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUInt: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, uint32_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eInt64: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, int64_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eUInt64: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, uint64_t, 1>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eFloat: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, float, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<true, float, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<true, float, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<true, float, 4>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case carb::flatcache::BaseDataType::eDouble: {
                switch (componentCount) {
                    case 1: {
                        return printAttributeValue<true, double, 1>(primPath, name);
                    }
                    case 2: {
                        return printAttributeValue<true, double, 2>(primPath, name);
                    }
                    case 3: {
                        return printAttributeValue<true, double, 3>(primPath, name);
                    }
                    case 4: {
                        return printAttributeValue<true, double, 4>(primPath, name);
                    }
                    case 6: {
                        return printAttributeValue<true, double, 6>(primPath, name);
                    }
                    case 9: {
                        return printAttributeValue<true, double, 9>(primPath, name);
                    }
                    case 16: {
                        return printAttributeValue<true, double, 16>(primPath, name);
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    return TYPE_NOT_SUPPORTED_STRING;
}

} // namespace

std::string printFabricStage() {
    std::stringstream stream;

    auto stageInProgress = UsdUtil::getFabricStageInProgress();

    // For extra debugging. This gets printed to the console.
    stageInProgress.printBucketNames();

    // This returns ALL the buckets
    const auto& buckets = stageInProgress.findPrims({});

    for (size_t bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
        const auto& attributes = stageInProgress.getAttributeNamesAndTypes(buckets, bucketId);
        const auto& primPaths = stageInProgress.getPathArray(buckets, bucketId);

        for (const auto& primPath : primPaths) {
            const auto primPathString = primPath.getText();
            const auto primPathUint64 = carb::flatcache::PathC(primPath).path;

            stream << fmt::format("Prim: {} ({})\n", primPathString, primPathUint64);
            stream << fmt::format("  Attributes:\n");

            for (const auto& attribute : attributes) {
                const auto attributeName = attribute.name.getText();
                const auto attributeType = attribute.type.getTypeName();
                const auto attributeBaseType = attribute.type.baseType;
                const auto attributeValue = printAttributeValue(primPath, attribute);

                stream << fmt::format("    Attribute: {}\n", attributeName);
                stream << fmt::format("      Type: {}\n", attributeType);

                if (attributeBaseType != carb::flatcache::BaseDataType::eTag) {
                    stream << fmt::format("      Value: {}\n", attributeValue);
                }
            }
        }
    }

    return stream.str();
}

namespace {
void addFabricPrim(
    const pxr::SdfPath& path,
    const pxr::VtArray<pxr::GfVec3f>& positions,
    const pxr::VtArray<int>& indices,
    const pxr::VtArray<int>& faceVertexCounts,
    const pxr::GfRange3d& localExtent,
    const glm::dmat4& matrix,
    bool doubleSided,
    const glm::fvec3& color) {
    auto sip = UsdUtil::getFabricStageInProgress();
    const auto geomPathFabric = carb::flatcache::Path(carb::flatcache::asInt(path));

    const auto [worldPosition, worldOrientation, worldScale] = UsdUtil::glmToUsdMatrixDecomposed(matrix);
    const auto worldExtent = UsdUtil::computeWorldExtent(localExtent, matrix);

    sip.createPrim(geomPathFabric);

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
    attributes.addAttribute(FabricTypes::_worldPosition, FabricTokens::_worldPosition);
    attributes.addAttribute(FabricTypes::_worldOrientation, FabricTokens::_worldOrientation);
    attributes.addAttribute(FabricTypes::_worldScale, FabricTokens::_worldScale);
    attributes.addAttribute(FabricTypes::doubleSided, FabricTokens::doubleSided);
    attributes.addAttribute(FabricTypes::subdivisionScheme, FabricTokens::subdivisionScheme);

    attributes.createAttributes(geomPathFabric);

    size_t primvarsCount = 0;

    const size_t primvarIndexDisplayColor = primvarsCount++;

    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::faceVertexCounts, faceVertexCounts.size());
    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::faceVertexIndices, indices.size());
    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::points, positions.size());
    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::primvars, primvarsCount);
    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::primvarInterpolations, primvarsCount);
    sip.setArrayAttributeSize(geomPathFabric, FabricTokens::primvars_displayColor, 1);

    // clang-format off
    auto faceVertexCountsFabric = sip.getArrayAttributeWr<int>(geomPathFabric, FabricTokens::faceVertexCounts);
    auto faceVertexIndicesFabric = sip.getArrayAttributeWr<int>(geomPathFabric, FabricTokens::faceVertexIndices);
    auto pointsFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(geomPathFabric, FabricTokens::points);
    auto localExtentFabric = sip.getAttributeWr<pxr::GfRange3d>(geomPathFabric, FabricTokens::_localExtent);
    auto worldExtentFabric = sip.getAttributeWr<pxr::GfRange3d>(geomPathFabric, FabricTokens::_worldExtent);
    auto worldVisibilityFabric = sip.getAttributeWr<bool>(geomPathFabric, FabricTokens::_worldVisibility);
    auto primvarsFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(geomPathFabric, FabricTokens::primvars);
    auto primvarInterpolationsFabric = sip.getArrayAttributeWr<carb::flatcache::Token>(geomPathFabric, FabricTokens::primvarInterpolations);
    auto displayColorFabric = sip.getArrayAttributeWr<pxr::GfVec3f>(geomPathFabric, FabricTokens::primvars_displayColor);
    auto worldPositionFabric = sip.getAttributeWr<pxr::GfVec3d>(geomPathFabric, FabricTokens::_worldPosition);
    auto worldOrientationFabric = sip.getAttributeWr<pxr::GfQuatf>(geomPathFabric, FabricTokens::_worldOrientation);
    auto worldScaleFabric = sip.getAttributeWr<pxr::GfVec3f>(geomPathFabric, FabricTokens::_worldScale);
    auto doubleSidedFabric = sip.getAttributeWr<bool>(geomPathFabric, FabricTokens::doubleSided);
    auto subdivisionSchemeFabric = sip.getAttributeWr<carb::flatcache::Token>(geomPathFabric, FabricTokens::subdivisionScheme);
    // clang-format on

    std::copy(faceVertexCounts.begin(), faceVertexCounts.end(), faceVertexCountsFabric.begin());
    std::copy(indices.begin(), indices.end(), faceVertexIndicesFabric.begin());
    std::copy(positions.begin(), positions.end(), pointsFabric.begin());

    *worldVisibilityFabric = true;
    primvarsFabric[primvarIndexDisplayColor] = FabricTokens::primvars_displayColor;
    primvarInterpolationsFabric[primvarIndexDisplayColor] = FabricTokens::constant;
    *worldPositionFabric = worldPosition;
    *worldOrientationFabric = worldOrientation;
    *worldScaleFabric = worldScale;
    *doubleSidedFabric = doubleSided;
    *subdivisionSchemeFabric = FabricTokens::none;
    *localExtentFabric = localExtent;
    *worldExtentFabric = worldExtent;

    displayColorFabric[0] = pxr::GfVec3f(color.x, color.y, color.z);
}

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

void removeFabricPrim(const pxr::SdfPath& path) {
    auto sip = UsdUtil::getFabricStageInProgress();

    sip.destroyPrim(carb::flatcache::asInt(path));

    deletePrimsFabric({path});
}

std::vector<pxr::SdfPath> paths;
int count = 0;

} // namespace

void addManyCubes() {
    for (int i = 0; i < 100; i++) {
        const auto path = pxr::SdfPath(fmt::format("/cube_{}", count++));
        const auto positions = pxr::VtArray<pxr::GfVec3f>{
            pxr::GfVec3f(1.0f, 1.0f, -1.0f),   pxr::GfVec3f(1.0f, 1.0f, -1.0f),   pxr::GfVec3f(1.0f, 1.0f, -1.0f),
            pxr::GfVec3f(1.0f, -1.0f, -1.0f),  pxr::GfVec3f(1.0f, -1.0f, -1.0f),  pxr::GfVec3f(1.0f, -1.0f, -1.0f),
            pxr::GfVec3f(1.0f, 1.0f, 1.0f),    pxr::GfVec3f(1.0f, 1.0f, 1.0f),    pxr::GfVec3f(1.0f, 1.0f, 1.0f),
            pxr::GfVec3f(1.0f, -1.0f, 1.0f),   pxr::GfVec3f(1.0f, -1.0f, 1.0f),   pxr::GfVec3f(1.0f, -1.0f, 1.0f),
            pxr::GfVec3f(-1.0f, 1.0f, -1.0f),  pxr::GfVec3f(-1.0f, 1.0f, -1.0f),  pxr::GfVec3f(-1.0f, 1.0f, -1.0f),
            pxr::GfVec3f(-1.0f, -1.0f, -1.0f), pxr::GfVec3f(-1.0f, -1.0f, -1.0f), pxr::GfVec3f(-1.0f, -1.0f, -1.0f),
            pxr::GfVec3f(-1.0f, 1.0f, 1.0f),   pxr::GfVec3f(-1.0f, 1.0f, 1.0f),   pxr::GfVec3f(-1.0f, 1.0f, 1.0f),
            pxr::GfVec3f(-1.0f, -1.0f, 1.0f),  pxr::GfVec3f(-1.0f, -1.0f, 1.0f),  pxr::GfVec3f(-1.0f, -1.0f, 1.0f),
        };
        const auto indices = pxr::VtArray<int>{
            14, 7,  1, 6, 23, 10, 18, 15, 21, 3, 22, 16, 2, 11, 5,  13, 4, 17,
            14, 20, 7, 6, 19, 23, 18, 12, 15, 3, 9,  22, 2, 8,  11, 13, 0, 4,
        };
        const auto faceVertexCounts = pxr::VtArray<int>{3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
        const auto localExtent = pxr::GfRange3d(pxr::GfVec3d(-1.0, -1.0, -1.0), pxr::GfVec3d(1.0, 1.0, 1.0));

        auto scale = glm::dvec3(glm::linearRand(20.0, 50.0));
        auto translation =
            glm::dvec3(glm::linearRand(-100.0, 100.0), glm::linearRand(-100.0, 100.0), glm::linearRand(-100.0, 100.0));
        auto scaleMatrix = glm::scale(glm::dmat4(1.0), scale);
        auto translationMatrix = glm::translate(glm::dmat4(1.0), translation);
        auto matrix = translationMatrix * scaleMatrix;

        const auto doubleSided = false;
        const auto color =
            glm::fvec3(glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f), glm::linearRand(0.0f, 1.0f));

        addFabricPrim(path, positions, indices, faceVertexCounts, localExtent, matrix, doubleSided, color);

        paths.push_back(path);
    }
}

void removeManyCubes() {
    for (const auto& path : paths) {
        removeFabricPrim(path);
    }

    paths.clear();
}

} // namespace cesium::omniverse::FabricUtil
