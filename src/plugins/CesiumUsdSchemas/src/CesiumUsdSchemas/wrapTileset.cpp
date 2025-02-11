#include ".//tileset.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyContainerConversions.h"
#include "pxr/base/tf/pyResultConversions.h"
#include "pxr/base/tf/pyUtils.h"
#include "pxr/base/tf/wrapTypeHelpers.h"

#include <boost/python.hpp>

#include <string>

using namespace boost::python;

PXR_NAMESPACE_USING_DIRECTIVE

namespace {

#define WRAP_CUSTOM                                                     \
    template <class Cls> static void _CustomWrapCode(Cls &_class)

// fwd decl.
WRAP_CUSTOM;

        
static UsdAttribute
_CreateSourceTypeAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSourceTypeAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Token), writeSparsely);
}
        
static UsdAttribute
_CreateUrlAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateUrlAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}
        
static UsdAttribute
_CreateIonAssetIdAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateIonAssetIdAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Int64), writeSparsely);
}
        
static UsdAttribute
_CreateIonAccessTokenAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateIonAccessTokenAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}
        
static UsdAttribute
_CreateMaximumScreenSpaceErrorAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaximumScreenSpaceErrorAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreatePreloadAncestorsAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreatePreloadAncestorsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreatePreloadSiblingsAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreatePreloadSiblingsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateForbidHolesAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateForbidHolesAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateMaximumSimultaneousTileLoadsAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaximumSimultaneousTileLoadsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->UInt), writeSparsely);
}
        
static UsdAttribute
_CreateMaximumCachedBytesAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMaximumCachedBytesAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->UInt64), writeSparsely);
}
        
static UsdAttribute
_CreateLoadingDescendantLimitAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateLoadingDescendantLimitAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->UInt), writeSparsely);
}
        
static UsdAttribute
_CreateEnableFrustumCullingAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateEnableFrustumCullingAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateEnableFogCullingAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateEnableFogCullingAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateEnforceCulledScreenSpaceErrorAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateEnforceCulledScreenSpaceErrorAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateCulledScreenSpaceErrorAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCulledScreenSpaceErrorAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}
        
static UsdAttribute
_CreateSuspendUpdateAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSuspendUpdateAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateSmoothNormalsAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateSmoothNormalsAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateShowCreditsOnScreenAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateShowCreditsOnScreenAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateMainThreadLoadingTimeLimitAttr(CesiumTileset &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateMainThreadLoadingTimeLimitAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Float), writeSparsely);
}

static std::string
_Repr(const CesiumTileset &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "CesiumUsdSchemas.Tileset(%s)",
        primRepr.c_str());
}

} // anonymous namespace

void wrapCesiumTileset()
{
    typedef CesiumTileset This;

    class_<This, bases<UsdGeomGprim> >
        cls("Tileset");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("Define", &This::Define, (arg("stage"), arg("path")))
        .staticmethod("Define")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetSourceTypeAttr",
             &This::GetSourceTypeAttr)
        .def("CreateSourceTypeAttr",
             &_CreateSourceTypeAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetUrlAttr",
             &This::GetUrlAttr)
        .def("CreateUrlAttr",
             &_CreateUrlAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetIonAssetIdAttr",
             &This::GetIonAssetIdAttr)
        .def("CreateIonAssetIdAttr",
             &_CreateIonAssetIdAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetIonAccessTokenAttr",
             &This::GetIonAccessTokenAttr)
        .def("CreateIonAccessTokenAttr",
             &_CreateIonAccessTokenAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaximumScreenSpaceErrorAttr",
             &This::GetMaximumScreenSpaceErrorAttr)
        .def("CreateMaximumScreenSpaceErrorAttr",
             &_CreateMaximumScreenSpaceErrorAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetPreloadAncestorsAttr",
             &This::GetPreloadAncestorsAttr)
        .def("CreatePreloadAncestorsAttr",
             &_CreatePreloadAncestorsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetPreloadSiblingsAttr",
             &This::GetPreloadSiblingsAttr)
        .def("CreatePreloadSiblingsAttr",
             &_CreatePreloadSiblingsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetForbidHolesAttr",
             &This::GetForbidHolesAttr)
        .def("CreateForbidHolesAttr",
             &_CreateForbidHolesAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaximumSimultaneousTileLoadsAttr",
             &This::GetMaximumSimultaneousTileLoadsAttr)
        .def("CreateMaximumSimultaneousTileLoadsAttr",
             &_CreateMaximumSimultaneousTileLoadsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMaximumCachedBytesAttr",
             &This::GetMaximumCachedBytesAttr)
        .def("CreateMaximumCachedBytesAttr",
             &_CreateMaximumCachedBytesAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetLoadingDescendantLimitAttr",
             &This::GetLoadingDescendantLimitAttr)
        .def("CreateLoadingDescendantLimitAttr",
             &_CreateLoadingDescendantLimitAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetEnableFrustumCullingAttr",
             &This::GetEnableFrustumCullingAttr)
        .def("CreateEnableFrustumCullingAttr",
             &_CreateEnableFrustumCullingAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetEnableFogCullingAttr",
             &This::GetEnableFogCullingAttr)
        .def("CreateEnableFogCullingAttr",
             &_CreateEnableFogCullingAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetEnforceCulledScreenSpaceErrorAttr",
             &This::GetEnforceCulledScreenSpaceErrorAttr)
        .def("CreateEnforceCulledScreenSpaceErrorAttr",
             &_CreateEnforceCulledScreenSpaceErrorAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetCulledScreenSpaceErrorAttr",
             &This::GetCulledScreenSpaceErrorAttr)
        .def("CreateCulledScreenSpaceErrorAttr",
             &_CreateCulledScreenSpaceErrorAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSuspendUpdateAttr",
             &This::GetSuspendUpdateAttr)
        .def("CreateSuspendUpdateAttr",
             &_CreateSuspendUpdateAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetSmoothNormalsAttr",
             &This::GetSmoothNormalsAttr)
        .def("CreateSmoothNormalsAttr",
             &_CreateSmoothNormalsAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetShowCreditsOnScreenAttr",
             &This::GetShowCreditsOnScreenAttr)
        .def("CreateShowCreditsOnScreenAttr",
             &_CreateShowCreditsOnScreenAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetMainThreadLoadingTimeLimitAttr",
             &This::GetMainThreadLoadingTimeLimitAttr)
        .def("CreateMainThreadLoadingTimeLimitAttr",
             &_CreateMainThreadLoadingTimeLimitAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        
        .def("GetGeoreferenceBindingRel",
             &This::GetGeoreferenceBindingRel)
        .def("CreateGeoreferenceBindingRel",
             &This::CreateGeoreferenceBindingRel)
        
        .def("GetIonServerBindingRel",
             &This::GetIonServerBindingRel)
        .def("CreateIonServerBindingRel",
             &This::CreateIonServerBindingRel)
        .def("__repr__", ::_Repr)
    ;

    _CustomWrapCode(cls);
}

// ===================================================================== //
// Feel free to add custom code below this line, it will be preserved by 
// the code generator.  The entry point for your custom code should look
// minimally like the following:
//
// WRAP_CUSTOM {
//     _class
//         .def("MyCustomMethod", ...)
//     ;
// }
//
// Of course any other ancillary or support code may be provided.
// 
// Just remember to wrap code in the appropriate delimiters:
// 'namespace {', '}'.
//
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--

namespace {

WRAP_CUSTOM {
}

}
