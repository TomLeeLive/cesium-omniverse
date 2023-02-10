#include ".//data.h"
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
_CreateCesiumDefaultProjectTokenIdAttr(CesiumData &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCesiumDefaultProjectTokenIdAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}
        
static UsdAttribute
_CreateCesiumDefaultProjectTokenAttr(CesiumData &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCesiumDefaultProjectTokenAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->String), writeSparsely);
}
        
static UsdAttribute
_CreateCesiumGeoreferenceOriginAttr(CesiumData &self,
                                      object defaultVal, bool writeSparsely) {
    return self.CreateCesiumGeoreferenceOriginAttr(
        UsdPythonToSdfType(defaultVal, SdfValueTypeNames->Double3), writeSparsely);
}

static std::string
_Repr(const CesiumData &self)
{
    std::string primRepr = TfPyRepr(self.GetPrim());
    return TfStringPrintf(
        "Cesium.Data(%s)",
        primRepr.c_str());
}

} // anonymous namespace

void wrapCesiumData()
{
    typedef CesiumData This;

    class_<This, bases<UsdTyped> >
        cls("Data");

    cls
        .def(init<UsdPrim>(arg("prim")))
        .def(init<UsdSchemaBase const&>(arg("schemaObj")))
        .def(TfTypePythonClass())

        .def("Get", &This::Get, (arg("stage"), arg("path")))
        .staticmethod("Get")

        .def("GetSchemaAttributeNames",
             &This::GetSchemaAttributeNames,
             arg("includeInherited")=true,
             return_value_policy<TfPySequenceToList>())
        .staticmethod("GetSchemaAttributeNames")

        .def("_GetStaticTfType", (TfType const &(*)()) TfType::Find<This>,
             return_value_policy<return_by_value>())
        .staticmethod("_GetStaticTfType")

        .def(!self)

        
        .def("GetCesiumDefaultProjectTokenIdAttr",
             &This::GetCesiumDefaultProjectTokenIdAttr)
        .def("CreateCesiumDefaultProjectTokenIdAttr",
             &_CreateCesiumDefaultProjectTokenIdAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetCesiumDefaultProjectTokenAttr",
             &This::GetCesiumDefaultProjectTokenAttr)
        .def("CreateCesiumDefaultProjectTokenAttr",
             &_CreateCesiumDefaultProjectTokenAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))
        
        .def("GetCesiumGeoreferenceOriginAttr",
             &This::GetCesiumGeoreferenceOriginAttr)
        .def("CreateCesiumGeoreferenceOriginAttr",
             &_CreateCesiumGeoreferenceOriginAttr,
             (arg("defaultValue")=object(),
              arg("writeSparsely")=false))

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
