#ifndef CESIUM_TOKENS_H
#define CESIUM_TOKENS_H

/// \file cesium/tokens.h

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// 
// This is an automatically generated file (by usdGenSchema.py).
// Do not hand-edit!
// 
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "pxr/pxr.h"
#include ".//api.h"
#include "pxr/base/tf/staticData.h"
#include "pxr/base/tf/token.h"
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE


/// \class CesiumTokensType
///
/// \link CesiumTokens \endlink provides static, efficient
/// \link TfToken TfTokens\endlink for use in all public USD API.
///
/// These tokens are auto-generated from the module's schema, representing
/// property names, for when you need to fetch an attribute or relationship
/// directly by name, e.g. UsdPrim::GetAttribute(), in the most efficient
/// manner, and allow the compiler to verify that you spelled the name
/// correctly.
///
/// CesiumTokens also contains all of the \em allowedTokens values
/// declared for schema builtin attributes of 'token' scene description type.
/// Use CesiumTokens like so:
///
/// \code
///     gprim.GetMyTokenValuedAttr().Set(CesiumTokens->cesiumCulledScreenSpaceError);
/// \endcode
struct CesiumTokensType {
    CESIUM_API CesiumTokensType();
    /// \brief "cesium:culledScreenSpaceError"
    /// 
    /// CesiumTileset
    const TfToken cesiumCulledScreenSpaceError;
    /// \brief "cesium:debug:disableGeometryPool"
    /// 
    /// CesiumData
    const TfToken cesiumDebugDisableGeometryPool;
    /// \brief "cesium:debug:disableMaterialPool"
    /// 
    /// CesiumData
    const TfToken cesiumDebugDisableMaterialPool;
    /// \brief "cesium:debug:disableMaterials"
    /// 
    /// CesiumData
    const TfToken cesiumDebugDisableMaterials;
    /// \brief "cesium:debug:disableTextures"
    /// 
    /// CesiumData
    const TfToken cesiumDebugDisableTextures;
    /// \brief "cesium:debug:geometryPoolInitialCapacity"
    /// 
    /// CesiumData
    const TfToken cesiumDebugGeometryPoolInitialCapacity;
    /// \brief "cesium:debug:materialPoolInitialCapacity"
    /// 
    /// CesiumData
    const TfToken cesiumDebugMaterialPoolInitialCapacity;
    /// \brief "cesium:debug:randomColors"
    /// 
    /// CesiumData
    const TfToken cesiumDebugRandomColors;
    /// \brief "cesium:ecefToUsdTransform"
    /// 
    /// CesiumSession
    const TfToken cesiumEcefToUsdTransform;
    /// \brief "cesium:enableFogCulling"
    /// 
    /// CesiumTileset
    const TfToken cesiumEnableFogCulling;
    /// \brief "cesium:enableFrustumCulling"
    /// 
    /// CesiumTileset
    const TfToken cesiumEnableFrustumCulling;
    /// \brief "cesium:enforceCulledScreenSpaceError"
    /// 
    /// CesiumTileset
    const TfToken cesiumEnforceCulledScreenSpaceError;
    /// \brief "cesium:forbidHoles"
    /// 
    /// CesiumTileset
    const TfToken cesiumForbidHoles;
    /// \brief "cesium:georeferenceBinding"
    /// 
    /// CesiumTileset
    const TfToken cesiumGeoreferenceBinding;
    /// \brief "cesium:georeferenceOrigin:height"
    /// 
    /// CesiumGeoreference
    const TfToken cesiumGeoreferenceOriginHeight;
    /// \brief "cesium:georeferenceOrigin:latitude"
    /// 
    /// CesiumGeoreference
    const TfToken cesiumGeoreferenceOriginLatitude;
    /// \brief "cesium:georeferenceOrigin:longitude"
    /// 
    /// CesiumGeoreference
    const TfToken cesiumGeoreferenceOriginLongitude;
    /// \brief "cesium:ionAccessToken"
    /// 
    /// CesiumImagery, CesiumTileset
    const TfToken cesiumIonAccessToken;
    /// \brief "cesium:ionAssetId"
    /// 
    /// CesiumImagery, CesiumTileset
    const TfToken cesiumIonAssetId;
    /// \brief "cesium:loadingDescendantLimit"
    /// 
    /// CesiumTileset
    const TfToken cesiumLoadingDescendantLimit;
    /// \brief "cesium:maximumCachedBytes"
    /// 
    /// CesiumTileset
    const TfToken cesiumMaximumCachedBytes;
    /// \brief "cesium:maximumScreenSpaceError"
    /// 
    /// CesiumTileset
    const TfToken cesiumMaximumScreenSpaceError;
    /// \brief "cesium:maximumSimultaneousTileLoads"
    /// 
    /// CesiumTileset
    const TfToken cesiumMaximumSimultaneousTileLoads;
    /// \brief "cesium:preloadAncestors"
    /// 
    /// CesiumTileset
    const TfToken cesiumPreloadAncestors;
    /// \brief "cesium:preloadSiblings"
    /// 
    /// CesiumTileset
    const TfToken cesiumPreloadSiblings;
    /// \brief "cesium:projectDefaultIonAccessToken"
    /// 
    /// CesiumData
    const TfToken cesiumProjectDefaultIonAccessToken;
    /// \brief "cesium:projectDefaultIonAccessTokenId"
    /// 
    /// CesiumData
    const TfToken cesiumProjectDefaultIonAccessTokenId;
    /// \brief "cesium:showCreditsOnScreen"
    /// 
    /// CesiumImagery, CesiumTileset
    const TfToken cesiumShowCreditsOnScreen;
    /// \brief "cesium:smoothNormals"
    /// 
    /// CesiumTileset
    const TfToken cesiumSmoothNormals;
    /// \brief "cesium:sourceType"
    /// 
    /// CesiumTileset
    const TfToken cesiumSourceType;
    /// \brief "cesium:suspendUpdate"
    /// 
    /// CesiumTileset
    const TfToken cesiumSuspendUpdate;
    /// \brief "cesium:url"
    /// 
    /// CesiumTileset
    const TfToken cesiumUrl;
    /// \brief "ion"
    /// 
    /// Possible value for CesiumTileset::GetCesiumSourceTypeAttr(), Default value for CesiumTileset::GetCesiumSourceTypeAttr()
    const TfToken ion;
    /// \brief "url"
    /// 
    /// Possible value for CesiumTileset::GetCesiumSourceTypeAttr()
    const TfToken url;
    /// A vector of all of the tokens listed above.
    const std::vector<TfToken> allTokens;
};

/// \var CesiumTokens
///
/// A global variable with static, efficient \link TfToken TfTokens\endlink
/// for use in all public USD API.  \sa CesiumTokensType
extern CESIUM_API TfStaticData<CesiumTokensType> CesiumTokens;

PXR_NAMESPACE_CLOSE_SCOPE

#endif
