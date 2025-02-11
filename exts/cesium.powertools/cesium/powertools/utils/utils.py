import omni.usd
from omni.kit.viewport.utility import get_active_viewport
from pxr import Gf, UsdGeom, Sdf
import json
import carb.settings
import os
from cesium.omniverse.utils.cesium_interface import CesiumInterfaceManager


# Modified version of ScopedEdit in _build_viewport_cameras in omni.kit.widget.viewport
class ScopedEdit:
    def __init__(self, stage):
        self.__stage = stage
        edit_target = stage.GetEditTarget()
        edit_layer = edit_target.GetLayer()
        self.__edit_layer = stage.GetSessionLayer()
        self.__was_editable = self.__edit_layer.permissionToEdit
        if not self.__was_editable:
            self.__edit_layer.SetPermissionToEdit(True)
        if self.__edit_layer != edit_layer:
            stage.SetEditTarget(self.__edit_layer)
            self.__edit_target = edit_target
        else:
            self.__edit_target = None

    def __del__(self):
        if self.__edit_layer and not self.__was_editable:
            self.__edit_layer.SetPermissionToEdit(False)
            self.__edit_layer = None

        if self.__edit_target:
            self.__stage.SetEditTarget(self.__edit_target)
            self.__edit_target = None


def extend_far_plane():
    stage = omni.usd.get_context().get_stage()
    viewport = get_active_viewport()
    camera_path = viewport.get_active_camera()
    camera = UsdGeom.Camera.Get(stage, camera_path)
    assert camera.GetPrim().IsValid()

    scoped_edit = ScopedEdit(stage)  # noqa: F841
    camera.GetClippingRangeAttr().Set(Gf.Vec2f(1.0, 10000000000.0))


def save_carb_settings(powertools_extension_location: str):
    carb_settings_path = os.path.join(powertools_extension_location, "carb_settings.txt")
    with open(carb_settings_path, "w") as fh:
        fh.write(json.dumps(carb.settings.get_settings().get("/"), indent=2))


def save_fabric_stage(powertools_extension_location: str):
    with CesiumInterfaceManager() as interface:
        fabric_stage_path = os.path.join(powertools_extension_location, "fabric_stage.txt")
        with open(fabric_stage_path, "w") as fh:
            fh.write(interface.print_fabric_stage())


# Helper function to search for an attribute on a prim, or create it if not present
def get_or_create_attribute(prim, name, type):
    attribute = prim.GetAttribute(name)
    if not attribute:
        attribute = prim.CreateAttribute(name, type)
    return attribute


def set_sunstudy_from_georef():
    stage = omni.usd.get_context().get_stage()

    environment_prim = stage.GetPrimAtPath("/Environment")
    cesium_prim = stage.GetPrimAtPath("/CesiumGeoreference")

    lat_attr = get_or_create_attribute(environment_prim, "location:latitude", Sdf.ValueTypeNames.Float)
    lat_attr.Set(cesium_prim.GetAttribute("cesium:georeferenceOrigin:latitude").Get())

    long_attr = get_or_create_attribute(environment_prim, "location:longitude", Sdf.ValueTypeNames.Float)
    long_attr.Set(cesium_prim.GetAttribute("cesium:georeferenceOrigin:longitude").Get())

    north_attr = get_or_create_attribute(environment_prim, "location:north_orientation", Sdf.ValueTypeNames.Float)
    north_attr.Set(90.0)  # Always set to 90, otherwise the sun is at the wrong angle
