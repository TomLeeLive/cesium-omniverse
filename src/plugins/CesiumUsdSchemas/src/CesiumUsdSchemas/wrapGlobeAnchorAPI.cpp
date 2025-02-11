#include ".//globeAnchorAPI.h"
#include "pxr/usd/usd/schemaBase.h"

#include "pxr/usd/sdf/primSpec.h"

#include "pxr/usd/usd/pyConversions.h"
#include "pxr/base/tf/pyAnnotatedBoolResult.h"
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
_CreateAdjustOrientationForGlobeWhenMovingAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateAdjustOrientationForGlobeWhenMovingAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateDetectTransformChangesAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateDetectTransformChangesAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Bool), writeSparsely);
}
        
static UsdAttribute
_CreateGeographicCoordinateAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateGeographicCoordinateAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double3), writeSparsely);
}
        
static UsdAttribute
_CreatePositionAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreatePositionAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double3), writeSparsely);
}
        
static UsdAttribute
_CreateRotationAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateRotationAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double3), writeSparsely);
}
        
static UsdAttribute
_CreateScaleAttr(CesiumGlobeAnchorAPI &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateScaleAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double3), writeSparsely);
}

static std::string
_Repr(const CesiumGlobeAnchorAPI &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "CesiumUsdSchemas.GlobeAnchorAPI(%s)",
        primRepr.c_str());
}

struct CesiumGlobeAnchorAPI_CanApplyResult : 
    public TfPyAnnotatedBoolResult<std::string>
{
    CesiumGlobeAnchorAPI_CanApplyResult(bool val, std::string const &msg) :
        TfPyAnnotatedBoolResult<std::string>(val, msg) {}
};

static CesiumGlobeAnchorAPI_CanApplyResult
_WrapCanApply(const UsdPrim& prim)
{
    std::string whyNot;
    bool result = CesiumGlobeAnchorAPI::CanApply(prim, &whyNot);
    return CesiumGlobeAnchorAPI_CanApplyResult(result, whyNot);
}

} // anonymous namespace

void wrapCesiumGlobeAnchorAPI()
{
    typedef CesiumGlobeAnchorAPI This;

    CesiumGlobeAnchorAPI_CanApplyResult::Wrap<CesiumGlobeAnchorAPI_CanApplyResult>(
        "_CanApplyResult", "whyNot");

    class_<This, bases<UsdAPISchemaBase> >
        cls("GlobeAnchorAPI");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("CanApply", &_WrapCanApply, (arg("prim")))
        .staticmethod("CanApply")

        .def("Apply", &This::Apply, (arg("prim")))
        .staticmethod("Apply")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetAdjustOrientationForGlobeWhenMovingAttr",
             &This::GetAdjustOrientationForGlobeWhenMovingAttr)
        .def("CreateAdjustOrientationForGlobeWhenMovingAttr",
             &_CreateAdjustOrientationForGlobeWhenMovingAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetDetectTransformChangesAttr",
             &This::GetDetectTransformChangesAttr)
        .def("CreateDetectTransformChangesAttr",
             &_CreateDetectTransformChangesAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetGeographicCoordinateAttr",
             &This::GetGeographicCoordinateAttr)
        .def("CreateGeographicCoordinateAttr",
             &_CreateGeographicCoordinateAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetPositionAttr",
             &This::GetPositionAttr)
        .def("CreatePositionAttr",
             &_CreatePositionAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetRotationAttr",
             &This::GetRotationAttr)
        .def("CreateRotationAttr",
             &_CreateRotationAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetScaleAttr",
             &This::GetScaleAttr)
        .def("CreateScaleAttr",
             &_CreateScaleAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

        
        .def("GetGeoreferenceBindingRel",
             &This::GetGeoreferenceBindingRel)
        .def("CreateGeoreferenceBindingRel",
             &This::CreateGeoreferenceBindingRel)
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
