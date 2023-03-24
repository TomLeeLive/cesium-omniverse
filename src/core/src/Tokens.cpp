#include "cesium/omniverse/Tokens.h"

#include <carb/flatcache/FlatCacheUSD.h>
#include <pxr/base/tf/staticTokens.h>

// clang-format off
namespace pxr {

// Note: variable names should match the USD token names as closely as possible, with special characters converted to underscores
TF_DEFINE_PRIVATE_TOKENS(
    UsdTokens,
    (a)
    (add)
    (albedo_add)
    (b)
    (constant)
    (coord)
    (diffuse_color_constant)
    (diffuse_texture)
    (displacement)
    (doubleSided)
    (faceVertexCounts)
    (faceVertexIndices)
    (lookup_color)
    (Material)
    (materialId)
    (MaterialNetwork)
    (Mesh)
    (metallic_constant)
    (multiply)
    (none)
    (OmniPBR)
    (out)
    (points)
    (primvarInterpolations)
    (primvars)
    (reflection_roughness_constant)
    (Shader)
    (specular_level)
    (subdivisionScheme)
    (surface)
    (tex)
    (texture_coordinate_2d)
    (vertex)
    (visibility)
    (wrap_u)
    (wrap_v)
    (_cesium_localToEcefTransform)
    (_cesium_tileId)
    (_cesium_tilesetId)
    (_deletedPrims)
    (_localExtent)
    (_localMatrix)
    (_nodePaths)
    (_paramColorSpace)
    (_parameters)
    (_relationships_inputId)
    (_relationships_inputName)
    (_relationships_outputId)
    (_relationships_outputName)
    (_terminals)
    (_worldExtent)
    (_worldOrientation)
    (_worldPosition)
    (_worldScale)
    (_worldVisibility)
    ((add_float2_float2, "add(float2,float2)"))
    ((info_id, "info:id"))
    ((info_sourceAsset_subIdentifier, "info:sourceAsset:subIdentifier"))
    ((multiply_float2_float2, "multiply(float2,float2)"))
    ((nvidia_support_definitions_mdl, "nvidia/support_definitions.mdl"))
    ((OmniPBR_mdl, "OmniPBR.mdl"))
    ((primvars_displayColor, "primvars:displayColor"))
    ((primvars_normals, "primvars:normals"))
    ((primvars_st, "primvars:st"))
    ((_auto, "auto"))
    (extent)
    (UsdGeomMesh)
    (UsdGeomImageable)
    (UsdGeomXformable)
    (UsdTyped)
    (UsdSchemaBase)
    (UsdGeomPointBased)
    (UsdGeomGprim)
    (UsdGeomBoundable)
    (MeshSubdivision)
    (purpose)
    (geometry)
    (refinementLevel)
    (orientation)
    (rightHanded)
    ((TfType_Root, "TfType::_Root"))
    (_memUsed)
    (faceVaryingLinearInterpolation)
    (interpolateBoundary)
    (triangleSubdivisionRule)
    (cornerIndices)
    (creaseIndices)
    (creaseLengths)
    (subsetIndicesCount)
    (subsetIndices)
    (creaseSharpnesses)
    (cornerSharpnesses)
    (subsetId)
    (subsetMaterialId)
    (timeVaryingAttributes)
);
}

namespace cesium::omniverse::FabricTokens {
const carb::flatcache::TokenC a = carb::flatcache::asInt(pxr::UsdTokens->a);
const carb::flatcache::TokenC add = carb::flatcache::asInt(pxr::UsdTokens->add);
const carb::flatcache::TokenC add_float2_float2 = carb::flatcache::asInt(pxr::UsdTokens->add_float2_float2);
const carb::flatcache::TokenC albedo_add = carb::flatcache::asInt(pxr::UsdTokens->albedo_add);
const carb::flatcache::TokenC b = carb::flatcache::asInt(pxr::UsdTokens->b);
const carb::flatcache::TokenC constant = carb::flatcache::asInt(pxr::UsdTokens->constant);
const carb::flatcache::TokenC coord = carb::flatcache::asInt(pxr::UsdTokens->coord);
const carb::flatcache::TokenC diffuse_color_constant = carb::flatcache::asInt(pxr::UsdTokens->diffuse_color_constant);
const carb::flatcache::TokenC diffuse_texture = carb::flatcache::asInt(pxr::UsdTokens->diffuse_texture);
const carb::flatcache::TokenC displacement = carb::flatcache::asInt(pxr::UsdTokens->displacement);
const carb::flatcache::TokenC doubleSided = carb::flatcache::asInt(pxr::UsdTokens->doubleSided);
const carb::flatcache::TokenC faceVertexCounts = carb::flatcache::asInt(pxr::UsdTokens->faceVertexCounts);
const carb::flatcache::TokenC faceVertexIndices = carb::flatcache::asInt(pxr::UsdTokens->faceVertexIndices);
const carb::flatcache::TokenC info_id = carb::flatcache::asInt(pxr::UsdTokens->info_id);
const carb::flatcache::TokenC info_sourceAsset_subIdentifier = carb::flatcache::asInt(pxr::UsdTokens->info_sourceAsset_subIdentifier);
const carb::flatcache::TokenC lookup_color = carb::flatcache::asInt(pxr::UsdTokens->lookup_color);
const carb::flatcache::TokenC Material = carb::flatcache::asInt(pxr::UsdTokens->Material);
const carb::flatcache::TokenC materialId = carb::flatcache::asInt(pxr::UsdTokens->materialId);
const carb::flatcache::TokenC MaterialNetwork = carb::flatcache::asInt(pxr::UsdTokens->MaterialNetwork);
const carb::flatcache::TokenC Mesh = carb::flatcache::asInt(pxr::UsdTokens->Mesh);
const carb::flatcache::TokenC metallic_constant = carb::flatcache::asInt(pxr::UsdTokens->metallic_constant);
const carb::flatcache::TokenC multiply = carb::flatcache::asInt(pxr::UsdTokens->multiply);
const carb::flatcache::TokenC multiply_float2_float2 = carb::flatcache::asInt(pxr::UsdTokens->multiply_float2_float2);
const carb::flatcache::TokenC none = carb::flatcache::asInt(pxr::UsdTokens->none);
const carb::flatcache::TokenC nvidia_support_definitions_mdl = carb::flatcache::asInt(pxr::UsdTokens->nvidia_support_definitions_mdl);
const carb::flatcache::TokenC OmniPBR = carb::flatcache::asInt(pxr::UsdTokens->OmniPBR);
const carb::flatcache::TokenC OmniPBR_mdl = carb::flatcache::asInt(pxr::UsdTokens->OmniPBR_mdl);
const carb::flatcache::TokenC out = carb::flatcache::asInt(pxr::UsdTokens->out);
const carb::flatcache::TokenC points = carb::flatcache::asInt(pxr::UsdTokens->points);
const carb::flatcache::TokenC primvarInterpolations = carb::flatcache::asInt(pxr::UsdTokens->primvarInterpolations);
const carb::flatcache::TokenC primvars = carb::flatcache::asInt(pxr::UsdTokens->primvars);
const carb::flatcache::TokenC primvars_displayColor = carb::flatcache::asInt(pxr::UsdTokens->primvars_displayColor);
const carb::flatcache::TokenC primvars_normals = carb::flatcache::asInt(pxr::UsdTokens->primvars_normals);
const carb::flatcache::TokenC primvars_st = carb::flatcache::asInt(pxr::UsdTokens->primvars_st);
const carb::flatcache::TokenC reflection_roughness_constant = carb::flatcache::asInt(pxr::UsdTokens->reflection_roughness_constant);
const carb::flatcache::TokenC Shader = carb::flatcache::asInt(pxr::UsdTokens->Shader);
const carb::flatcache::TokenC specular_level = carb::flatcache::asInt(pxr::UsdTokens->specular_level);
const carb::flatcache::TokenC subdivisionScheme = carb::flatcache::asInt(pxr::UsdTokens->subdivisionScheme);
const carb::flatcache::TokenC surface = carb::flatcache::asInt(pxr::UsdTokens->surface);
const carb::flatcache::TokenC tex = carb::flatcache::asInt(pxr::UsdTokens->tex);
const carb::flatcache::TokenC texture_coordinate_2d = carb::flatcache::asInt(pxr::UsdTokens->texture_coordinate_2d);
const carb::flatcache::TokenC vertex = carb::flatcache::asInt(pxr::UsdTokens->vertex);
const carb::flatcache::TokenC visibility = carb::flatcache::asInt(pxr::UsdTokens->visibility);
const carb::flatcache::TokenC wrap_u = carb::flatcache::asInt(pxr::UsdTokens->wrap_u);
const carb::flatcache::TokenC wrap_v = carb::flatcache::asInt(pxr::UsdTokens->wrap_v);
const carb::flatcache::TokenC _auto = carb::flatcache::asInt(pxr::UsdTokens->_auto);
const carb::flatcache::TokenC _cesium_localToEcefTransform = carb::flatcache::asInt(pxr::UsdTokens->_cesium_localToEcefTransform);
const carb::flatcache::TokenC _cesium_tileId = carb::flatcache::asInt(pxr::UsdTokens->_cesium_tileId);
const carb::flatcache::TokenC _cesium_tilesetId = carb::flatcache::asInt(pxr::UsdTokens->_cesium_tilesetId);
const carb::flatcache::TokenC _deletedPrims = carb::flatcache::asInt(pxr::UsdTokens->_deletedPrims);
const carb::flatcache::TokenC _localExtent = carb::flatcache::asInt(pxr::UsdTokens->_localExtent);
const carb::flatcache::TokenC _localMatrix = carb::flatcache::asInt(pxr::UsdTokens->_localMatrix);
const carb::flatcache::TokenC _nodePaths = carb::flatcache::asInt(pxr::UsdTokens->_nodePaths);
const carb::flatcache::TokenC _paramColorSpace = carb::flatcache::asInt(pxr::UsdTokens->_paramColorSpace);
const carb::flatcache::TokenC _parameters = carb::flatcache::asInt(pxr::UsdTokens->_parameters);
const carb::flatcache::TokenC _relationships_inputId = carb::flatcache::asInt(pxr::UsdTokens->_relationships_inputId);
const carb::flatcache::TokenC _relationships_inputName = carb::flatcache::asInt(pxr::UsdTokens->_relationships_inputName);
const carb::flatcache::TokenC _relationships_outputId = carb::flatcache::asInt(pxr::UsdTokens->_relationships_outputId);
const carb::flatcache::TokenC _relationships_outputName = carb::flatcache::asInt(pxr::UsdTokens->_relationships_outputName);
const carb::flatcache::TokenC _terminals = carb::flatcache::asInt(pxr::UsdTokens->_terminals);
const carb::flatcache::TokenC _worldExtent = carb::flatcache::asInt(pxr::UsdTokens->_worldExtent);
const carb::flatcache::TokenC _worldOrientation = carb::flatcache::asInt(pxr::UsdTokens->_worldOrientation);
const carb::flatcache::TokenC _worldPosition = carb::flatcache::asInt(pxr::UsdTokens->_worldPosition);
const carb::flatcache::TokenC _worldScale = carb::flatcache::asInt(pxr::UsdTokens->_worldScale);
const carb::flatcache::TokenC _worldVisibility = carb::flatcache::asInt(pxr::UsdTokens->_worldVisibility);
const carb::flatcache::TokenC extent = carb::flatcache::asInt(pxr::UsdTokens->extent);
const carb::flatcache::TokenC UsdGeomMesh = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomMesh);
const carb::flatcache::TokenC UsdGeomImageable = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomImageable);
const carb::flatcache::TokenC UsdGeomXformable = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomXformable);
const carb::flatcache::TokenC UsdTyped = carb::flatcache::asInt(pxr::UsdTokens->UsdTyped);
const carb::flatcache::TokenC UsdSchemaBase = carb::flatcache::asInt(pxr::UsdTokens->UsdSchemaBase);
const carb::flatcache::TokenC UsdGeomPointBased = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomPointBased);
const carb::flatcache::TokenC UsdGeomGprim = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomGprim);
const carb::flatcache::TokenC UsdGeomBoundable = carb::flatcache::asInt(pxr::UsdTokens->UsdGeomBoundable);
const carb::flatcache::TokenC MeshSubdivision = carb::flatcache::asInt(pxr::UsdTokens->MeshSubdivision);
const carb::flatcache::TokenC purpose = carb::flatcache::asInt(pxr::UsdTokens->purpose);
const carb::flatcache::TokenC geometry = carb::flatcache::asInt(pxr::UsdTokens->geometry);
const carb::flatcache::TokenC refinementLevel = carb::flatcache::asInt(pxr::UsdTokens->refinementLevel);
const carb::flatcache::TokenC orientation = carb::flatcache::asInt(pxr::UsdTokens->orientation);
const carb::flatcache::TokenC rightHanded = carb::flatcache::asInt(pxr::UsdTokens->rightHanded);
const carb::flatcache::TokenC TfType_Root = carb::flatcache::asInt(pxr::UsdTokens->TfType_Root);
const carb::flatcache::TokenC _memUsed = carb::flatcache::asInt(pxr::UsdTokens->_memUsed);
const carb::flatcache::TokenC faceVaryingLinearInterpolation = carb::flatcache::asInt(pxr::UsdTokens->faceVaryingLinearInterpolation);
const carb::flatcache::TokenC interpolateBoundary = carb::flatcache::asInt(pxr::UsdTokens->interpolateBoundary);
const carb::flatcache::TokenC triangleSubdivisionRule = carb::flatcache::asInt(pxr::UsdTokens->triangleSubdivisionRule);
const carb::flatcache::TokenC cornerIndices = carb::flatcache::asInt(pxr::UsdTokens->cornerIndices);
const carb::flatcache::TokenC creaseIndices = carb::flatcache::asInt(pxr::UsdTokens->creaseIndices);
const carb::flatcache::TokenC creaseLengths = carb::flatcache::asInt(pxr::UsdTokens->creaseLengths);
const carb::flatcache::TokenC subsetIndicesCount = carb::flatcache::asInt(pxr::UsdTokens->subsetIndicesCount);
const carb::flatcache::TokenC subsetIndices = carb::flatcache::asInt(pxr::UsdTokens->subsetIndices);
const carb::flatcache::TokenC creaseSharpnesses = carb::flatcache::asInt(pxr::UsdTokens->creaseSharpnesses);
const carb::flatcache::TokenC cornerSharpnesses = carb::flatcache::asInt(pxr::UsdTokens->cornerSharpnesses);
const carb::flatcache::TokenC subsetId = carb::flatcache::asInt(pxr::UsdTokens->subsetId);
const carb::flatcache::TokenC subsetMaterialId = carb::flatcache::asInt(pxr::UsdTokens->subsetMaterialId);
const carb::flatcache::TokenC timeVaryingAttributes = carb::flatcache::asInt(pxr::UsdTokens->timeVaryingAttributes);
}

namespace cesium::omniverse::UsdTokens {
const pxr::TfToken& a = pxr::UsdTokens->a;
const pxr::TfToken& add = pxr::UsdTokens->add;
const pxr::TfToken& add_float2_float2 = pxr::UsdTokens->add_float2_float2;
const pxr::TfToken& albedo_add = pxr::UsdTokens->albedo_add;
const pxr::TfToken& b = pxr::UsdTokens->b;
const pxr::TfToken& constant = pxr::UsdTokens->constant;
const pxr::TfToken& coord = pxr::UsdTokens->coord;
const pxr::TfToken& diffuse_color_constant = pxr::UsdTokens->diffuse_color_constant;
const pxr::TfToken& diffuse_texture = pxr::UsdTokens->diffuse_texture;
const pxr::TfToken& displacement = pxr::UsdTokens->displacement;
const pxr::TfToken& doubleSided = pxr::UsdTokens->doubleSided;
const pxr::TfToken& faceVertexCounts = pxr::UsdTokens->faceVertexCounts;
const pxr::TfToken& faceVertexIndices = pxr::UsdTokens->faceVertexIndices;
const pxr::TfToken& info_id = pxr::UsdTokens->info_id;
const pxr::TfToken& info_sourceAsset_subIdentifier = pxr::UsdTokens->info_sourceAsset_subIdentifier;
const pxr::TfToken& lookup_color = pxr::UsdTokens->lookup_color;
const pxr::TfToken& Material = pxr::UsdTokens->Material;
const pxr::TfToken& materialId = pxr::UsdTokens->materialId;
const pxr::TfToken& MaterialNetwork = pxr::UsdTokens->MaterialNetwork;
const pxr::TfToken& Mesh = pxr::UsdTokens->Mesh;
const pxr::TfToken& metallic_constant = pxr::UsdTokens->metallic_constant;
const pxr::TfToken& multiply = pxr::UsdTokens->multiply;
const pxr::TfToken& multiply_float2_float2 = pxr::UsdTokens->multiply_float2_float2;
const pxr::TfToken& none = pxr::UsdTokens->none;
const pxr::TfToken& nvidia_support_definitions_mdl = pxr::UsdTokens->nvidia_support_definitions_mdl;
const pxr::TfToken& OmniPBR = pxr::UsdTokens->OmniPBR;
const pxr::TfToken& OmniPBR_mdl = pxr::UsdTokens->OmniPBR_mdl;
const pxr::TfToken& out = pxr::UsdTokens->out;
const pxr::TfToken& points = pxr::UsdTokens->points;
const pxr::TfToken& primvarInterpolations = pxr::UsdTokens->primvarInterpolations;
const pxr::TfToken& primvars = pxr::UsdTokens->primvars;
const pxr::TfToken& primvars_displayColor = pxr::UsdTokens->primvars_displayColor;
const pxr::TfToken& primvars_normals = pxr::UsdTokens->primvars_normals;
const pxr::TfToken& primvars_st = pxr::UsdTokens->primvars_st;
const pxr::TfToken& reflection_roughness_constant = pxr::UsdTokens->reflection_roughness_constant;
const pxr::TfToken& Shader = pxr::UsdTokens->Shader;
const pxr::TfToken& specular_level = pxr::UsdTokens->specular_level;
const pxr::TfToken& subdivisionScheme = pxr::UsdTokens->subdivisionScheme;
const pxr::TfToken& surface = pxr::UsdTokens->surface;
const pxr::TfToken& tex = pxr::UsdTokens->tex;
const pxr::TfToken& texture_coordinate_2d = pxr::UsdTokens->texture_coordinate_2d;
const pxr::TfToken& vertex = pxr::UsdTokens->vertex;
const pxr::TfToken& visibility = pxr::UsdTokens->visibility;
const pxr::TfToken& wrap_u = pxr::UsdTokens->wrap_u;
const pxr::TfToken& wrap_v = pxr::UsdTokens->wrap_v;
const pxr::TfToken& _auto = pxr::UsdTokens->_auto;
const pxr::TfToken& _cesium_localToEcefTransform = pxr::UsdTokens->_cesium_localToEcefTransform;
const pxr::TfToken& _cesium_tileId = pxr::UsdTokens->_cesium_tileId;
const pxr::TfToken& _cesium_tilesetId = pxr::UsdTokens->_cesium_tilesetId;
const pxr::TfToken& _deletedPrims = pxr::UsdTokens->_deletedPrims;
const pxr::TfToken& _localExtent = pxr::UsdTokens->_localExtent;
const pxr::TfToken& _localMatrix = pxr::UsdTokens->_localMatrix;
const pxr::TfToken& _nodePaths = pxr::UsdTokens->_nodePaths;
const pxr::TfToken& _paramColorSpace = pxr::UsdTokens->_paramColorSpace;
const pxr::TfToken& _parameters = pxr::UsdTokens->_parameters;
const pxr::TfToken& _relationships_inputId = pxr::UsdTokens->_relationships_inputId;
const pxr::TfToken& _relationships_inputName = pxr::UsdTokens->_relationships_inputName;
const pxr::TfToken& _relationships_outputId = pxr::UsdTokens->_relationships_outputId;
const pxr::TfToken& _relationships_outputName = pxr::UsdTokens->_relationships_outputName;
const pxr::TfToken& _terminals = pxr::UsdTokens->_terminals;
const pxr::TfToken& _worldExtent = pxr::UsdTokens->_worldExtent;
const pxr::TfToken& _worldOrientation = pxr::UsdTokens->_worldOrientation;
const pxr::TfToken& _worldPosition = pxr::UsdTokens->_worldPosition;
const pxr::TfToken& _worldScale = pxr::UsdTokens->_worldScale;
const pxr::TfToken& _worldVisibility = pxr::UsdTokens->_worldVisibility;
const pxr::TfToken& extent = pxr::UsdTokens->extent;
const pxr::TfToken& UsdGeomMesh = pxr::UsdTokens->UsdGeomMesh;
const pxr::TfToken& UsdGeomImageable = pxr::UsdTokens->UsdGeomImageable;
const pxr::TfToken& UsdGeomXformable = pxr::UsdTokens->UsdGeomXformable;
const pxr::TfToken& UsdTyped = pxr::UsdTokens->UsdTyped;
const pxr::TfToken& UsdSchemaBase = pxr::UsdTokens->UsdSchemaBase;
const pxr::TfToken& UsdGeomPointBased = pxr::UsdTokens->UsdGeomPointBased;
const pxr::TfToken& UsdGeomGprim = pxr::UsdTokens->UsdGeomGprim;
const pxr::TfToken& UsdGeomBoundable = pxr::UsdTokens->UsdGeomBoundable;
const pxr::TfToken& MeshSubdivision = pxr::UsdTokens->MeshSubdivision;
const pxr::TfToken& purpose = pxr::UsdTokens->purpose;
const pxr::TfToken& geometry = pxr::UsdTokens->geometry;
const pxr::TfToken& refinementLevel = pxr::UsdTokens->refinementLevel;
const pxr::TfToken& orientation = pxr::UsdTokens->orientation;
const pxr::TfToken& rightHanded = pxr::UsdTokens->rightHanded;
const pxr::TfToken& TfType_Root = pxr::UsdTokens->TfType_Root;
const pxr::TfToken& _memUsed = pxr::UsdTokens->_memUsed;
const pxr::TfToken& faceVaryingLinearInterpolation = pxr::UsdTokens->faceVaryingLinearInterpolation;
const pxr::TfToken& interpolateBoundary = pxr::UsdTokens->interpolateBoundary;
const pxr::TfToken& triangleSubdivisionRule = pxr::UsdTokens->triangleSubdivisionRule;
const pxr::TfToken& cornerIndices = pxr::UsdTokens->cornerIndices;
const pxr::TfToken& creaseIndices = pxr::UsdTokens->creaseIndices;
const pxr::TfToken& creaseLengths = pxr::UsdTokens->creaseLengths;
const pxr::TfToken& subsetIndicesCount = pxr::UsdTokens->subsetIndicesCount;
const pxr::TfToken& subsetIndices = pxr::UsdTokens->subsetIndices;
const pxr::TfToken& creaseSharpnesses = pxr::UsdTokens->creaseSharpnesses;
const pxr::TfToken& cornerSharpnesses = pxr::UsdTokens->cornerSharpnesses;
const pxr::TfToken& subsetId = pxr::UsdTokens->subsetId;
const pxr::TfToken& subsetMaterialId = pxr::UsdTokens->subsetMaterialId;
const pxr::TfToken& timeVaryingAttributes = pxr::UsdTokens->timeVaryingAttributes;
}
// clang-format on
