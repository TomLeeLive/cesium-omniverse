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
}

namespace cesium::omniverse::UsdTokens {
const usdrt::TfToken a = usdrt::TfToken(pxr::UsdTokens->a.GetText());
const usdrt::TfToken add = usdrt::TfToken(pxr::UsdTokens->add.GetText());
const usdrt::TfToken add_float2_float2 = usdrt::TfToken(pxr::UsdTokens->add_float2_float2.GetText());
const usdrt::TfToken albedo_add = usdrt::TfToken(pxr::UsdTokens->albedo_add.GetText());
const usdrt::TfToken b = usdrt::TfToken(pxr::UsdTokens->b.GetText());
const usdrt::TfToken constant = usdrt::TfToken(pxr::UsdTokens->constant.GetText());
const usdrt::TfToken coord = usdrt::TfToken(pxr::UsdTokens->coord.GetText());
const usdrt::TfToken diffuse_color_constant = usdrt::TfToken(pxr::UsdTokens->diffuse_color_constant.GetText());
const usdrt::TfToken diffuse_texture = usdrt::TfToken(pxr::UsdTokens->diffuse_texture.GetText());
const usdrt::TfToken displacement = usdrt::TfToken(pxr::UsdTokens->displacement.GetText());
const usdrt::TfToken doubleSided = usdrt::TfToken(pxr::UsdTokens->doubleSided.GetText());
const usdrt::TfToken faceVertexCounts = usdrt::TfToken(pxr::UsdTokens->faceVertexCounts.GetText());
const usdrt::TfToken faceVertexIndices = usdrt::TfToken(pxr::UsdTokens->faceVertexIndices.GetText());
const usdrt::TfToken info_id = usdrt::TfToken(pxr::UsdTokens->info_id.GetText());
const usdrt::TfToken info_sourceAsset_subIdentifier = usdrt::TfToken(pxr::UsdTokens->info_sourceAsset_subIdentifier.GetText());
const usdrt::TfToken lookup_color = usdrt::TfToken(pxr::UsdTokens->lookup_color.GetText());
const usdrt::TfToken Material = usdrt::TfToken(pxr::UsdTokens->Material.GetText());
const usdrt::TfToken materialId = usdrt::TfToken(pxr::UsdTokens->materialId.GetText());
const usdrt::TfToken MaterialNetwork = usdrt::TfToken(pxr::UsdTokens->MaterialNetwork.GetText());
const usdrt::TfToken Mesh = usdrt::TfToken(pxr::UsdTokens->Mesh.GetText());
const usdrt::TfToken metallic_constant = usdrt::TfToken(pxr::UsdTokens->metallic_constant.GetText());
const usdrt::TfToken multiply = usdrt::TfToken(pxr::UsdTokens->multiply.GetText());
const usdrt::TfToken multiply_float2_float2 = usdrt::TfToken(pxr::UsdTokens->multiply_float2_float2.GetText());
const usdrt::TfToken none = usdrt::TfToken(pxr::UsdTokens->none.GetText());
const usdrt::TfToken nvidia_support_definitions_mdl = usdrt::TfToken(pxr::UsdTokens->nvidia_support_definitions_mdl.GetText());
const usdrt::TfToken OmniPBR = usdrt::TfToken(pxr::UsdTokens->OmniPBR.GetText());
const usdrt::TfToken OmniPBR_mdl = usdrt::TfToken(pxr::UsdTokens->OmniPBR_mdl.GetText());
const usdrt::TfToken out = usdrt::TfToken(pxr::UsdTokens->out.GetText());
const usdrt::TfToken points = usdrt::TfToken(pxr::UsdTokens->points.GetText());
const usdrt::TfToken primvarInterpolations = usdrt::TfToken(pxr::UsdTokens->primvarInterpolations.GetText());
const usdrt::TfToken primvars = usdrt::TfToken(pxr::UsdTokens->primvars.GetText());
const usdrt::TfToken primvars_displayColor = usdrt::TfToken(pxr::UsdTokens->primvars_displayColor.GetText());
const usdrt::TfToken primvars_normals = usdrt::TfToken(pxr::UsdTokens->primvars_normals.GetText());
const usdrt::TfToken primvars_st = usdrt::TfToken(pxr::UsdTokens->primvars_st.GetText());
const usdrt::TfToken reflection_roughness_constant = usdrt::TfToken(pxr::UsdTokens->reflection_roughness_constant.GetText());
const usdrt::TfToken Shader = usdrt::TfToken(pxr::UsdTokens->Shader.GetText());
const usdrt::TfToken specular_level = usdrt::TfToken(pxr::UsdTokens->specular_level.GetText());
const usdrt::TfToken subdivisionScheme = usdrt::TfToken(pxr::UsdTokens->subdivisionScheme.GetText());
const usdrt::TfToken surface = usdrt::TfToken(pxr::UsdTokens->surface.GetText());
const usdrt::TfToken tex = usdrt::TfToken(pxr::UsdTokens->tex.GetText());
const usdrt::TfToken texture_coordinate_2d = usdrt::TfToken(pxr::UsdTokens->texture_coordinate_2d.GetText());
const usdrt::TfToken vertex = usdrt::TfToken(pxr::UsdTokens->vertex.GetText());
const usdrt::TfToken visibility = usdrt::TfToken(pxr::UsdTokens->visibility.GetText());
const usdrt::TfToken wrap_u = usdrt::TfToken(pxr::UsdTokens->wrap_u.GetText());
const usdrt::TfToken wrap_v = usdrt::TfToken(pxr::UsdTokens->wrap_v.GetText());
const usdrt::TfToken _auto = usdrt::TfToken(pxr::UsdTokens->_auto.GetText());
const usdrt::TfToken _cesium_localToEcefTransform = usdrt::TfToken(pxr::UsdTokens->_cesium_localToEcefTransform.GetText());
const usdrt::TfToken _cesium_tileId = usdrt::TfToken(pxr::UsdTokens->_cesium_tileId.GetText());
const usdrt::TfToken _cesium_tilesetId = usdrt::TfToken(pxr::UsdTokens->_cesium_tilesetId.GetText());
const usdrt::TfToken _deletedPrims = usdrt::TfToken(pxr::UsdTokens->_deletedPrims.GetText());
const usdrt::TfToken _localExtent = usdrt::TfToken(pxr::UsdTokens->_localExtent.GetText());
const usdrt::TfToken _localMatrix = usdrt::TfToken(pxr::UsdTokens->_localMatrix.GetText());
const usdrt::TfToken _nodePaths = usdrt::TfToken(pxr::UsdTokens->_nodePaths.GetText());
const usdrt::TfToken _paramColorSpace = usdrt::TfToken(pxr::UsdTokens->_paramColorSpace.GetText());
const usdrt::TfToken _parameters = usdrt::TfToken(pxr::UsdTokens->_parameters.GetText());
const usdrt::TfToken _relationships_inputId = usdrt::TfToken(pxr::UsdTokens->_relationships_inputId.GetText());
const usdrt::TfToken _relationships_inputName = usdrt::TfToken(pxr::UsdTokens->_relationships_inputName.GetText());
const usdrt::TfToken _relationships_outputId = usdrt::TfToken(pxr::UsdTokens->_relationships_outputId.GetText());
const usdrt::TfToken _relationships_outputName = usdrt::TfToken(pxr::UsdTokens->_relationships_outputName.GetText());
const usdrt::TfToken _terminals = usdrt::TfToken(pxr::UsdTokens->_terminals.GetText());
const usdrt::TfToken _worldExtent = usdrt::TfToken(pxr::UsdTokens->_worldExtent.GetText());
const usdrt::TfToken _worldOrientation = usdrt::TfToken(pxr::UsdTokens->_worldOrientation.GetText());
const usdrt::TfToken _worldPosition = usdrt::TfToken(pxr::UsdTokens->_worldPosition.GetText());
const usdrt::TfToken _worldScale = usdrt::TfToken(pxr::UsdTokens->_worldScale.GetText());
const usdrt::TfToken _worldVisibility = usdrt::TfToken(pxr::UsdTokens->_worldVisibility.GetText());
}

// clang-format on
