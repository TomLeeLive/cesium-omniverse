#define CARB_EXPORTS

#include "cesium/omniverse/CesiumOmniverse.h"

#include "CesiumUsdSchemas/data.h"

#include "cesium/omniverse/OmniTileset.h"

#include <carb/PluginUtils.h>
#include <carb/flatcache/FlatCacheUSD.h>
#include <carb/flatcache/StageWithHistory.h>
#include <pxr/usd/usd/stageCache.h>
#include <pxr/usd/usdUtils/stageCache.h>
#include <usdrt/scenegraph/base/gf/range3d.h>
#include <usdrt/scenegraph/base/gf/vec3f.h>
#include <usdrt/scenegraph/usd/sdf/types.h>
#include <usdrt/scenegraph/usd/usd/stage.h>

#include <numeric>
#include <sstream>
#include <unordered_map>

namespace cesium::omniverse {

namespace {
int currentId = 0;
std::unordered_map<int, std::unique_ptr<OmniTileset>> tilesets;

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

class CesiumOmniversePlugin : public ICesiumOmniverseInterface {
  protected:
    void initialize(const char* cesiumMemLocation) noexcept override {
        OmniTileset::init(cesiumMemLocation);
    }

    void finalize() noexcept {
        OmniTileset::shutdown();
    }

    void addCesiumData(long stageId, const char* ionToken) noexcept override {
        const auto& stage = pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
        pxr::UsdPrim cesiumDataPrim = stage->DefinePrim(pxr::SdfPath("/Cesium"));
        pxr::CesiumData cesiumData(cesiumDataPrim);
        auto ionTokenAttr = cesiumData.CreateIonTokenAttr(pxr::VtValue(""));

        if (strlen(ionToken) != 0) {
            ionTokenAttr.Set(ionToken);
        }
    }

    int addTilesetUrl(long stageId, const char* url) noexcept override {
        const int tilesetId = currentId++;
        const auto& stage = pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
        tilesets.insert({tilesetId, std::make_unique<OmniTileset>(stage, url)});
        return tilesetId;
    }

    int addTilesetIon(long stageId, int64_t ionId, const char* ionToken) noexcept override {
        const int tilesetId = currentId++;
        const auto& stage = pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));
        tilesets.insert({tilesetId, std::make_unique<OmniTileset>(stage, ionId, ionToken)});
        return tilesetId;
    }

    void removeTileset(int tileset) noexcept override {
        tilesets.erase(tileset);
    }

    void addIonRasterOverlay(int tileset, const char* name, int64_t ionId, const char* ionToken) noexcept override {
        const auto iter = tilesets.find(tileset);
        if (iter != tilesets.end()) {
            iter->second->addIonRasterOverlay(name, ionId, ionToken);
        }
    }

    void updateFrame(
        int tileset,
        const pxr::GfMatrix4d& viewMatrix,
        const pxr::GfMatrix4d& projMatrix,
        double width,
        double height) noexcept override {
        const auto iter = tilesets.find(tileset);
        if (iter != tilesets.end()) {
            iter->second->updateFrame(viewMatrix, projMatrix, width, height);
        }
    }

    void setGeoreferenceOrigin(double longitude, double latitude, double height) noexcept override {
        Georeference::instance().setOrigin(CesiumGeospatial::Ellipsoid::WGS84.cartographicToCartesian(
            CesiumGeospatial::Cartographic(glm::radians(longitude), glm::radians(latitude), height)));
    }

    void addCubeUsdrt(long stageId) noexcept override {
        const auto& stage = usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});

        // Create a cube prim.
        const usdrt::SdfPath primPath("/example_prim_usdrt");
        const usdrt::UsdPrim prim = stage->DefinePrim(primPath, usdrt::TfToken("Mesh"));

        const usdrt::VtArray<int> faceVertexCounts = {4, 4, 4, 4, 4, 4};
        const usdrt::VtArray<int> faceVertexIndices = {0, 1, 3, 2, 2, 3, 7, 6, 6, 7, 5, 4,
                                                       4, 5, 1, 0, 2, 6, 4, 0, 7, 3, 1, 5};

        const usdrt::VtArray<usdrt::GfVec3f> points = {
            usdrt::GfVec3f(-1.0, -1.0, -1.0),
            usdrt::GfVec3f(-1.0, -1.0, 1.0),
            usdrt::GfVec3f(-1.0, 1.0, -1.0),
            usdrt::GfVec3f(-1.0, 1.0, 1.0),
            usdrt::GfVec3f(1.0, -1.0, -1.0),
            usdrt::GfVec3f(1.0, -1.0, 1.0),
            usdrt::GfVec3f(1.0, 1.0, -1.0),
            usdrt::GfVec3f(1.0, 1.0, 1.0)};

        usdrt::VtArray<usdrt::GfVec3f> displayColor = {usdrt::GfVec3f(1.0, 0.0, 0.0)};

        // clang-format off
        prim.CreateAttribute(usdrt::TfToken("faceVertexCounts"), usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexCounts);
        prim.CreateAttribute(usdrt::TfToken("faceVertexIndices"), usdrt::SdfValueTypeNames->IntArray, false).Set(faceVertexIndices);
        prim.CreateAttribute(usdrt::TfToken("points"), usdrt::SdfValueTypeNames->Point3fArray, false).Set(points);
        prim.CreateAttribute(usdrt::TfToken("primvars:displayColor"), usdrt::SdfValueTypeNames->Color3fArray, false).Set(displayColor);
        prim.CreateAttribute(usdrt::TfToken("_worldExtent"), usdrt::SdfValueTypeNames->Range3d, false).Set(usdrt::GfRange3d(usdrt::GfVec3d(-1.0, -1.0, -1.0), usdrt::GfVec3d(1.0, 1.0, 1.0)));

        // For Create 2022.3.1 you need to have at least one primvar on your Mesh, even if it does nothing, and two
        // new TokenArray attributes, "primvars", and "primvarInterpolations", which are used internally by Fabric
        // Scene Delegate. This is a workaround until UsdGeomMesh and UsdGeomPrimvarsAPI become available in USDRT.
        const usdrt::VtArray<carb::flatcache::TokenC> primvars = {carb::flatcache::TokenC(usdrt::TfToken("primvars:displayColor"))};
        const usdrt::VtArray<carb::flatcache::TokenC> primvarInterp = {carb::flatcache::TokenC(usdrt::TfToken("constant"))};

        prim.CreateAttribute(usdrt::TfToken("primvars"), usdrt::SdfValueTypeNames->TokenArray, false).Set(primvars);
        prim.CreateAttribute(usdrt::TfToken("primvarInterpolations"), usdrt::SdfValueTypeNames->TokenArray, false).Set(primvarInterp);
        // clang-format on
    }

    void addCubeUsd(long stageId) noexcept override {
        const auto& stage = pxr::UsdUtilsStageCache::Get().Find(pxr::UsdStageCache::Id::FromLongInt(stageId));

        // Create a cube prim.
        const pxr::SdfPath primPath("/example_prim_usd");
        const pxr::UsdPrim prim = stage->DefinePrim(primPath, pxr::TfToken("Mesh"));

        const pxr::VtArray<int> faceVertexCounts = {4, 4, 4, 4, 4, 4};
        const pxr::VtArray<int> faceVertexIndices = {0, 1, 3, 2, 2, 3, 7, 6, 6, 7, 5, 4,
                                                     4, 5, 1, 0, 2, 6, 4, 0, 7, 3, 1, 5};

        const pxr::VtArray<pxr::GfVec3f> points = {
            pxr::GfVec3f(-1.0, -1.0, -1.0),
            pxr::GfVec3f(-1.0, -1.0, 1.0),
            pxr::GfVec3f(-1.0, 1.0, -1.0),
            pxr::GfVec3f(-1.0, 1.0, 1.0),
            pxr::GfVec3f(1.0, -1.0, -1.0),
            pxr::GfVec3f(1.0, -1.0, 1.0),
            pxr::GfVec3f(1.0, 1.0, -1.0),
            pxr::GfVec3f(1.0, 1.0, 1.0)};

        pxr::VtArray<pxr::GfVec3f> displayColor = {pxr::GfVec3f(1.0, 0.0, 0.0)};

        // clang-format off
        prim.CreateAttribute(pxr::TfToken("faceVertexCounts"), pxr::SdfValueTypeNames->IntArray).Set(faceVertexCounts);
        prim.CreateAttribute(pxr::TfToken("faceVertexIndices"), pxr::SdfValueTypeNames->IntArray).Set(faceVertexIndices);
        prim.CreateAttribute(pxr::TfToken("points"), pxr::SdfValueTypeNames->Point3fArray).Set(points);
        prim.CreateAttribute(pxr::TfToken("primvars:displayColor"), pxr::SdfValueTypeNames->Color3fArray).Set(displayColor);
        // clang-format on
    }

    void addCubeFabric(long stageId) noexcept override {
        const auto& stage = usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});
        const auto stageInProgressId = stage->GetStageInProgressId();
        auto stageInProgress = carb::flatcache::StageInProgress(stageInProgressId);

        carb::flatcache::Path primPath("/example_prim_fabric");
        carb::flatcache::Token faceVertexCountsToken("faceVertexCounts");
        carb::flatcache::Token faceVertexIndicesToken("faceVertexIndices");
        carb::flatcache::Token pointsToken("points");
        carb::flatcache::Token worldExtentToken("_worldExtent");

        stageInProgress.createPrim(primPath);
        stageInProgress.createAttribute(
            primPath,
            faceVertexCountsToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone));
        stageInProgress.createAttribute(
            primPath,
            faceVertexIndicesToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eInt, 1, 1, carb::flatcache::AttributeRole::eNone));

        stageInProgress.createAttribute(
            primPath,
            pointsToken,
            carb::flatcache::Type(
                carb::flatcache::BaseDataType::eFloat, 3, 1, carb::flatcache::AttributeRole::ePosition));

        stageInProgress.createAttribute(
            primPath,
            worldExtentToken,
            carb::flatcache::Type(carb::flatcache::BaseDataType::eDouble, 6, 0, carb::flatcache::AttributeRole::eNone));

        stageInProgress.setArrayAttributeSize(primPath, faceVertexCountsToken, 6);
        stageInProgress.setArrayAttributeSize(primPath, faceVertexIndicesToken, 24);
        stageInProgress.setArrayAttributeSize(primPath, pointsToken, 8);

        auto faceVertexCounts = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexCountsToken);
        auto faceVertexIndices = stageInProgress.getArrayAttributeWr<int>(primPath, faceVertexIndicesToken);
        auto points = stageInProgress.getArrayAttributeWr<usdrt::GfVec3f>(primPath, pointsToken);
        auto worldExtent = stageInProgress.getAttributeWr<usdrt::GfRange3d>(primPath, worldExtentToken);

        faceVertexCounts[0] = 4;
        faceVertexCounts[1] = 4;
        faceVertexCounts[2] = 4;
        faceVertexCounts[3] = 4;
        faceVertexCounts[4] = 4;
        faceVertexCounts[5] = 4;

        faceVertexIndices[0] = 0;
        faceVertexIndices[1] = 1;
        faceVertexIndices[2] = 3;
        faceVertexIndices[3] = 2;
        faceVertexIndices[4] = 2;
        faceVertexIndices[5] = 3;
        faceVertexIndices[6] = 7;
        faceVertexIndices[7] = 6;
        faceVertexIndices[8] = 6;
        faceVertexIndices[9] = 7;
        faceVertexIndices[10] = 5;
        faceVertexIndices[11] = 4;
        faceVertexIndices[12] = 4;
        faceVertexIndices[13] = 5;
        faceVertexIndices[14] = 1;
        faceVertexIndices[15] = 0;
        faceVertexIndices[16] = 2;
        faceVertexIndices[17] = 6;
        faceVertexIndices[18] = 4;
        faceVertexIndices[19] = 0;
        faceVertexIndices[20] = 7;
        faceVertexIndices[21] = 3;
        faceVertexIndices[22] = 1;
        faceVertexIndices[23] = 5;

        points[0] = usdrt::GfVec3f(-1.0, -1.0, -1.0);
        points[1] = usdrt::GfVec3f(-1.0, -1.0, 1.0);
        points[2] = usdrt::GfVec3f(-1.0, 1.0, -1.0);
        points[3] = usdrt::GfVec3f(-1.0, 1.0, 1.0);
        points[4] = usdrt::GfVec3f(1.0, -1.0, -1.0);
        points[5] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[6] = usdrt::GfVec3f(1.0, -1.0, 1.0);
        points[7] = usdrt::GfVec3f(1.0, 1.0, 1.0);

        worldExtent->SetMin(usdrt::GfVec3d(-1.0, -1.0, -1.0));
        worldExtent->SetMax(usdrt::GfVec3d(1.0, 1.0, 1.0));
    }

    void removeCubeUsdrt(long stageId) noexcept override {
        const auto stage = usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});
        const auto primPath = usdrt::SdfPath("/example_prim_usdrt");

        const auto& prim = stage->GetPrimAtPath(primPath);
        if (prim.IsValid()) {
            int i = 0;
            (void)i;
        }

        stage->RemovePrim(primPath);

        // Prims removed from Fabric need special handling for their removal to be reflected in the Hydra render index
        // This workaround may not be needed in future Kit versions
        usdrt::UsdAttribute deletedPrimsAttribute =
            stage->GetAttributeAtPath(usdrt::SdfPath("/TempChangeTracking._deletedPrims"));

        if (!deletedPrimsAttribute.IsValid()) {
            return;
        }

        usdrt::VtArray<uint64_t> result;
        result.push_back(carb::flatcache::PathC(primPath).path);
        deletedPrimsAttribute.Set(result);
    }

    std::string printUsdrtStage(long stageId) noexcept override {
        const auto& stage = usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});

        // Notes aboute Traverse () and PrimRange from the USDRT docs:
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
        // return printUsdrtPrim(printUsdrtPrim, root, true);
    }

    std::string printFabricStage(long stageId) noexcept override {
        std::stringstream stream;

        const auto& stage = usdrt::UsdStage::Attach(carb::flatcache::UsdStageId{static_cast<uint64_t>(stageId)});
        const auto& stageInProgressId = stage->GetStageInProgressId();
        auto sip = carb::flatcache::StageInProgress(stageInProgressId);

        // For extra debugging
        sip.printBucketNames();

        struct AttributeInfo {
            std::string name;
            bool isArray;
        };

        std::vector<std::string> types = {"Mesh"};

        for (const auto& primTokenString : types) {
            carb::flatcache::Type primType(
                carb::flatcache::BaseDataType::eTag, 1, 0, carb::flatcache::AttributeRole::ePrimTypeName);
            carb::flatcache::Token primToken(primTokenString.c_str());

            const auto& buckets = sip.findPrims({carb::flatcache::AttrNameAndType(primType, primToken)});

            for (uint64_t bucketId = 0; bucketId < buckets.bucketCount(); bucketId++) {
                const auto& attributes = sip.getAttributeNamesAndTypes(buckets, bucketId);

                const auto primPaths = sip.getPathArray(buckets, bucketId);

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

                        // Not all cases are handled here; but has good coverage over SdfValueTypeNames which is what
                        // we expect to see from USDRT prims.
                        if (arrayDepth == 0) {
                            switch (baseType) {
                                case carb::flatcache::BaseDataType::eBool: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue = printAttributeValue<false, bool, 1>(sip, primPath, name);
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case carb::flatcache::BaseDataType::eUChar: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue =
                                                printAttributeValue<false, uint8_t, 1>(sip, primPath, name);
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case carb::flatcache::BaseDataType::eInt: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue =
                                                printAttributeValue<false, int32_t, 1>(sip, primPath, name);
                                            break;
                                        }
                                        case 2: {
                                            attributeValue =
                                                printAttributeValue<false, int32_t, 2>(sip, primPath, name);
                                            break;
                                        }
                                        case 3: {
                                            attributeValue =
                                                printAttributeValue<false, int32_t, 3>(sip, primPath, name);
                                            break;
                                        }
                                        case 4: {
                                            attributeValue =
                                                printAttributeValue<false, int32_t, 4>(sip, primPath, name);
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case carb::flatcache::BaseDataType::eUInt: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue =
                                                printAttributeValue<false, uint32_t, 1>(sip, primPath, name);
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case carb::flatcache::BaseDataType::eInt64: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue =
                                                printAttributeValue<false, int64_t, 1>(sip, primPath, name);
                                            break;
                                        }
                                    }
                                    break;
                                }
                                case carb::flatcache::BaseDataType::eUInt64: {
                                    switch (componentCount) {
                                        case 1: {
                                            attributeValue =
                                                printAttributeValue<false, uint64_t, 1>(sip, primPath, name);
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
                                            attributeValue =
                                                printAttributeValue<false, double, 16>(sip, primPath, name);
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
                                            attributeValue =
                                                printAttributeValue<true, uint32_t, 1>(sip, primPath, name);
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
                                            attributeValue =
                                                printAttributeValue<true, uint64_t, 1>(sip, primPath, name);
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
};

} // namespace cesium::omniverse

const struct carb::PluginImplDesc pluginImplDesc = {
    "cesium.omniverse.plugin",
    "Cesium Omniverse Carbonite Plugin.",
    "Cesium",
    carb::PluginHotReload::eDisabled,
    "dev"};

CARB_PLUGIN_IMPL(pluginImplDesc, cesium::omniverse::CesiumOmniversePlugin)

void fillInterface([[maybe_unused]] cesium::omniverse::CesiumOmniversePlugin& iface) {}
