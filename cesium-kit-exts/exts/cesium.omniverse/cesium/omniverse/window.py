from .bindings.cesium.omniverse import CesiumOmniversePythonBindings as CesiumOmniverse

import omni.ui as ui
from pxr import Gf, Sdf
import omni.usd
from omni.kit.viewport.utility import get_active_viewport_camera_path, get_active_viewport
import omni.kit.app as omni_app
import carb.settings as omni_settings


class CesiumOmniverseWindow(ui.Window):
    def __init__(self, title: str, **kwargs):
        super().__init__(title, **kwargs)

        # Set the function that is called to build widgets when the window is visible
        self.frame.set_build_fn(self._build_fn)

        print("[cesium.omniverse] CesiumOmniverse startup")
        CesiumOmniverse.startup()
        self._tilesets = []

    def destroy(self):
        # It will destroy all the children
        super().destroy()

        print("[cesium.omniverse] CesiumOmniverse shutdown")
        CesiumOmniverse.shutdown()

    def _build_fn(self):
        def remove_sphere():
            stage = omni.usd.get_context().get_stage()
            prim = stage.GetPrimAtPath("/World/Sphere")
            prim.GetAttribute("visibility").Set("visible")
            stage.RemovePrim("/World/Sphere")

        def create_sphere():
            stage = omni.usd.get_context().get_stage()
            sphere = omni.usd.UsdGeom.Sphere.Define(stage, "/World/Sphere")
            sphere.CreateDisplayColorAttr([Gf.Vec3f(1.0, 1.0, 1.0)])
            sphere.GetDisplayOpacityAttr().Set([1.0])
            sphere.GetRadiusAttr().Set(100.0)
            sphere.GetPrim().CreateAttribute("refinementEnableOverride", Sdf.ValueTypeNames.Bool)
            sphere.GetPrim().CreateAttribute("refinementLevel", Sdf.ValueTypeNames.Int)
            sphere.GetPrim().GetAttribute("refinementEnableOverride").Set(True)
            sphere.GetPrim().GetAttribute("refinementLevel").Set(2)

        def get_camera_params():
            viewport = get_active_viewport()
            camera_path = get_active_viewport_camera_path()
            print(camera_path)
            print(type(viewport.resolution))
            print(type(viewport.view.GetInverse()))
            print(viewport.projection)

        def on_update_frame(e):
            viewport = get_active_viewport()
            for tileset in self._tilesets:
                tileset.updateFrame(
                    viewport.view, viewport.projection, float(viewport.resolution[0]), float(viewport.resolution[1])
                )

        def create_update_frame():
            app = omni_app.get_app()
            omni_settings.get_settings().set("/rtx/hydra/TBNFrameMode", 1)
            self._subscription_handle = app.get_update_event_stream().create_subscription_to_pop(
                on_update_frame, name="cesium_update_frame"
            )

        def stop_update_frame():
            self._subscription_handle = None

        def create_tileset():
            stage = omni.usd.get_context().get_stage()
            self._tilesets.append(
                CesiumOmniverse.OmniTileset(
                    stage,
                    1,
                    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
                )
            )

        with ui.VStack():
            ui.Button("Create Sphere", clicked_fn=lambda: create_sphere())
            ui.Button("Remove Sphere", clicked_fn=lambda: remove_sphere())
            ui.Button("Get Camera", clicked_fn=lambda: get_camera_params())
            ui.Button("Update Frame", clicked_fn=lambda: create_update_frame())
            ui.Button("Stop Update Frame", clicked_fn=lambda: stop_update_frame())
            ui.Button("Create Tileset", clicked_fn=lambda: create_tileset())
