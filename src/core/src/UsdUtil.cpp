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

namespace cesium::omniverse {

pxr::UsdStageRefPtr getUsdStage(long stageId) {
    return pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
}

usdrt::UsdStageRefPtr getUsdrtStage(long stageId) {
    return usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});
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

void updatePrimTransforms(long stageId, int tilesetId, const glm::dmat4& ecefToUsdTransform) {
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

void updatePrimVisibility(long stageId, int tilesetId, const glm::dmat4& ecefToUsdTransform) {
    (void)stageId;
    (void)tilesetId;
    (void)ecefToUsdTransform;
    // TODO
}

namespace {
template <typename T>
std::string printAttributeValuePtr(const T* const values, const uint64_t elementCount, const uint64_t componentCount) {
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

template <typename T> std::string printAttributeValue(const usdrt::VtArray<T>& values, const uint64_t componentCount) {
    return printAttributeValuePtr(values.data(), values.size(), componentCount);
}

template <typename T> std::string printAttributeValue(const gsl::span<T>& values) {
    const uint64_t elementCount = values.size();
    const uint64_t componentCount = std::tuple_size<T>::value;
    return printAttributeValuePtr<T::value_type>(values.front().data(), elementCount, componentCount);
}

template <bool IsArray, typename BaseType, uint64_t ComponentCount>
std::string printAttributeValue(
    carb::flatcache::StageInProgress& stageInProgress,
    const carb::flatcache::Path& primPath,
    const carb::flatcache::Token& attributeName) {
    using ElementType = std::array<BaseType, ComponentCount>;

    if constexpr (IsArray) {
        const auto values = stageInProgress.getArrayAttributeRd<ElementType>(primPath, attributeName);
        return printAttributeValue(values);
    } else {
        const auto value = stageInProgress.getAttributeRd<ElementType>(primPath, attributeName);
        assert(value != nullptr);
        const auto values = gsl::span<const ElementType>(value, 1);
        return printAttributeValue(values);
    }
}

std::string printUsdrtPrim(const usdrt::UsdPrim& prim, const bool printChildren) {
    std::stringstream stream;

    const auto& name = prim.GetName().GetString();
    const auto& primPath = prim.GetPrimPath().GetString();
    const auto& typeName = prim.GetTypeName().GetString();
    const auto& attributes = prim.GetAttributes();
    const auto& children = prim.GetChildren();

    const uint64_t tabSize = 2;
    const uint64_t depth = std::count(primPath.begin(), primPath.end(), '/');
    const std::string primSpaces(depth * tabSize, ' ');
    const std::string primInfoSpaces((depth + 1) * tabSize, ' ');
    const std::string attributeSpaces((depth + 2) * tabSize, ' ');
    const std::string attributeInfoSpaces((depth + 3) * tabSize, ' ');

    const bool printPrim = primPath != "/";

    if (printPrim) {
        stream << fmt::format("{}Prim: {}\n", primSpaces, primPath);
        stream << fmt::format("{}Name: {}\n", primInfoSpaces, name);
        stream << fmt::format("{}Type: {}\n", primInfoSpaces, typeName);

        if (!attributes.empty()) {
            stream << fmt::format("{}Attributes:\n", primInfoSpaces);
        }

        for (const auto& attribute : attributes) {
            const auto& attributeName = attribute.GetName().GetString();
            const auto& attributeBaseName = attribute.GetBaseName().GetString();
            const auto& attributePath = attribute.GetPath().GetString();
            const auto& attributeNamespace = attribute.GetNamespace().GetString();
            const auto& attributeTypeName = attribute.GetTypeName().GetAsString();

            stream << fmt::format("{}Attribute: {}\n", attributeSpaces, attributeName);
            stream << fmt::format("{}Base Name: {}\n", attributeInfoSpaces, attributeBaseName);
            stream << fmt::format("{}Path: {}\n", attributeInfoSpaces, attributePath);
            stream << fmt::format("{}Namespace: {}\n", attributeInfoSpaces, attributeNamespace);
            stream << fmt::format("{}Type: {}\n", attributeInfoSpaces, attributeTypeName);

            if (attribute.HasValue()) {
                const auto attributeType = carb::flatcache::Type(attribute.GetTypeName().GetAsTypeC());
                const auto baseType = attributeType.baseType;
                const auto componentCount = attributeType.componentCount;

                std::string attributeValue;

                switch (baseType) {
                    case carb::flatcache::BaseDataType::eBool: {
                        usdrt::VtArray<bool> values;
                        attribute.Get<bool>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eUChar: {
                        usdrt::VtArray<uint8_t> values;
                        attribute.Get<uint8_t>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eInt: {
                        usdrt::VtArray<int32_t> values;
                        attribute.Get<int32_t>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eUInt: {
                        usdrt::VtArray<uint32_t> values;
                        attribute.Get<uint32_t>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eInt64: {
                        usdrt::VtArray<int64_t> values;
                        attribute.Get<int64_t>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eUInt64: {
                        usdrt::VtArray<uint64_t> values;
                        attribute.Get<uint64_t>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eFloat: {
                        usdrt::VtArray<float> values;
                        attribute.Get<float>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    case carb::flatcache::BaseDataType::eDouble: {
                        usdrt::VtArray<double> values;
                        attribute.Get<double>(&values);
                        attributeValue = printAttributeValue(values, componentCount);
                        break;
                    }
                    default: {
                        attributeValue = "[Type not supported]";
                        break;
                    }
                }

                if (attributeValue == "") {
                    attributeValue = "[Value not in Fabric]";
                }

                stream << fmt::format("{}Value: {}\n", attributeInfoSpaces, attributeValue);
            }
        }

        if (printChildren && !children.empty()) {
            stream << fmt::format("{}Children: {}\n", primInfoSpaces, typeName);
        }
    }

    if (printChildren) {
        for (const auto& child : prim.GetChildren()) {
            stream << printUsdrtPrim(child, printChildren);
        }
    }

    return stream.str();
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

    // These are the types of prins we want to print
    std::vector<std::string> types = {"Mesh"};

    for (const auto& primTokenString : types) {
        carb::flatcache::Type primType(
            carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);
        carb::flatcache::Token primToken(primTokenString.c_str());

        const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(primType, primToken)});

        for (uint64_t bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
            const auto& attributes = sip.getAttributeNamesAndTypes(buckets, bucketId);
            const auto& primPaths = sip.getPathArray(buckets, bucketId);

            for (const auto& primPath : primPaths) {
                const std::string primPathString = primPath.getText();
                const uint64_t tabSize = 2;
                const uint64_t depth = std::count(primPathString.begin(), primPathString.end(), '/');
                const std::string primSpaces(depth * tabSize, ' ');
                const std::string primInfoSpaces((depth + 1) * tabSize, ' ');
                const std::string attributeSpaces((depth + 2) * tabSize, ' ');
                const std::string attributeInfoSpaces((depth + 3) * tabSize, ' ');

                stream << fmt::format("{}Prim: {}\n", primSpaces, primPathString);
                stream << fmt::format("{}Type: {}\n", primInfoSpaces, primTokenString);
                stream << fmt::format("{}Attributes:\n", primInfoSpaces);

                for (const auto& attribute : attributes) {
                    const auto attributeType = attribute.type;
                    const auto baseType = attributeType.baseType;
                    const auto componentCount = attributeType.componentCount;
                    const auto name = attribute.name;
                    const auto arrayDepth = attributeType.arrayDepth;
                    const auto attributeNameString = name.getString();
                    const auto attributeTypeString = attributeType.getTypeName();

                    std::string attributeValue;

                    // These switch statements should cover all the attribute types we expect to see used by USDRT
                    // prims. See SdfValueTypeNames for the full list. Fabric allows for many more attribute types
                    // but we don't really care at the moment since we're always interfacing with Fabric via USDRT.
                    if (arrayDepth == 0) {
                        switch (baseType) {
                            case carb::flatcache::BaseDataType::eBool: {
                                switch (componentCount) {
                                    case 1: {
                                        // TODO: check that bool works
                                        attributeValue = printAttributeValue<false, bool, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUChar: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, uint8_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eInt: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, int32_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<false, int32_t, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<false, int32_t, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<false, int32_t, 4>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUInt: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, uint32_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eInt64: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, int64_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUInt64: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, uint64_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eFloat: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, float, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<false, float, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<false, float, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<false, float, 4>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eDouble: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<false, double, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<false, double, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<false, double, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<false, double, 4>(sip, primPath, name);
                                        break;
                                    }
                                    case 6: {
                                        attributeValue = printAttributeValue<false, double, 6>(sip, primPath, name);
                                        break;
                                    }
                                    case 9: {
                                        attributeValue = printAttributeValue<false, double, 9>(sip, primPath, name);
                                        break;
                                    }
                                    case 16: {
                                        attributeValue = printAttributeValue<false, double, 16>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    } else if (arrayDepth == 1) {
                        switch (baseType) {
                            case carb::flatcache::BaseDataType::eBool: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, bool, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUChar: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, uint8_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eInt: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, int32_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<true, int32_t, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<true, int32_t, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<true, int32_t, 4>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUInt: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, uint32_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eInt64: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, int64_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eUInt64: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, uint64_t, 1>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eFloat: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, float, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<true, float, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<true, float, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<true, float, 4>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                            case carb::flatcache::BaseDataType::eDouble: {
                                switch (componentCount) {
                                    case 1: {
                                        attributeValue = printAttributeValue<true, double, 1>(sip, primPath, name);
                                        break;
                                    }
                                    case 2: {
                                        attributeValue = printAttributeValue<true, double, 2>(sip, primPath, name);
                                        break;
                                    }
                                    case 3: {
                                        attributeValue = printAttributeValue<true, double, 3>(sip, primPath, name);
                                        break;
                                    }
                                    case 4: {
                                        attributeValue = printAttributeValue<true, double, 4>(sip, primPath, name);
                                        break;
                                    }
                                    case 6: {
                                        attributeValue = printAttributeValue<true, double, 6>(sip, primPath, name);
                                        break;
                                    }
                                    case 9: {
                                        attributeValue = printAttributeValue<true, double, 9>(sip, primPath, name);
                                        break;
                                    }
                                    case 16: {
                                        attributeValue = printAttributeValue<true, double, 16>(sip, primPath, name);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }

                    if (attributeValue == "") {
                        attributeValue = "[Type not supported]";
                    }

                    stream << fmt::format("{}Attribute: {}\n", attributeSpaces, attributeNameString);
                    stream << fmt::format("{}Type: {}\n", attributeInfoSpaces, attributeTypeString);
                    stream << fmt::format("{}Value: {}\n", attributeInfoSpaces, attributeValue);
                }
            }
        }
    }

    return stream.str();
}

} // namespace cesium::omniverse
