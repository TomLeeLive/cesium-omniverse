import asyncio

from carb.events._events import ISubscription
import carb.settings as omni_settings
from enum import Enum
import logging
import omni.ui as ui
import omni.usd
from omni.kit.viewport.utility import get_active_viewport, get_active_viewport_camera_path
import omni.kit.app as omni_app
import omni.kit.commands as omni_commands
from pxr import Gf, Sdf
import asyncio
from ..bindings import ICesiumOmniverseInterface
from ..utils.utils import wait_n_frames
from .fabric import get_fabric_data_for_prim


class Tileset(Enum):
    """Possible tilesets for use with Cesium for Omniverse."""

    CESIUM_WORLD_TERRAIN = 0
    BING_MAPS = 1
    CAPE_CANAVERAL = 2
    BING_MAPS_WITH_STAGE_TOKEN = 3


class CesiumOmniverseDebugWindow(ui.Window):
    WINDOW_NAME = "Cesium Debugging"
    MENU_PATH = f"Window/Cesium/{WINDOW_NAME}"

    _subscription_handle: ISubscription = None
    _logger: logging.Logger
    _cesium_omniverse_interface: ICesiumOmniverseInterface = None
    _cesium_message_field: ui.SimpleStringModel = None

    def __init__(self, cesium_omniverse_interface: ICesiumOmniverseInterface, title: str, **kwargs):
        super().__init__(title, **kwargs)

        self._logger = logging.getLogger(__name__)
        self._cesium_omniverse_interface = cesium_omniverse_interface

        # Set the function that is called to build widgets when the window is visible
        self.frame.set_build_fn(self._build_fn)

    def destroy_subscription(self):
        """Unsubscribes from the subscription handler for frame updates."""

        if self._subscription_handle is not None:
            self._subscription_handle.unsubscribe()
            self._subscription_handle = None

    def destroy(self):
        # It will destroy all the children
        self.destroy_subscription()
        super().destroy()

    def update_far_plane(self):
        """Sets the Far Plane to a very high number."""

        stage = omni.usd.get_context().get_stage()
        if stage is None:
            return

        camera_prim = stage.GetPrimAtPath(get_active_viewport_camera_path())
        omni_commands.execute(
            "ChangeProperty",
            prop_path=Sdf.Path("/OmniverseKit_Persp.clippingRange"),
            value=Gf.Vec2f(1.0, 10000000000.0),
            prev=camera_prim.GetAttribute("clippingRange").Get(),
        )

    def _build_fn(self):
        """Builds out the UI buttons and their handlers."""

        def on_update_frame(e):
            """Actions performed on each frame update."""

            viewport = get_active_viewport()

            stage_id = omni.usd.get_context().get_stage_id()

            self._cesium_omniverse_interface.updateFrame(
                stage_id,
                viewport.view,
                viewport.projection,
                float(viewport.resolution[0]),
                float(viewport.resolution[1]),
            )

        def start_update_frame():
            """Starts updating the frame, resulting in tileset updates."""

            app = omni_app.get_app()
            omni_settings.get_settings().set("/rtx/hydra/TBNFrameMode", 1)
            # Disabling Texture Streaming is a workaround for issues with Kit 104.1. We should remove this as soon as
            #   the issue is fixed.
            omni_settings.get_settings().set("/rtx-transient/resourcemanager/enableTextureStreaming", False)
            self._subscription_handle = app.get_update_event_stream().create_subscription_to_pop(
                on_update_frame, name="cesium_update_frame"
            )

        def stop_update_frame():
            """Stops updating the frame, thereby stopping tileset updates."""

            self.destroy_subscription()

        def add_maxar_3d_surface_model():
            """Adds the Maxar data of Cape Canaveral to the stage."""

            stage_id = omni.usd.get_context().get_stage_id()

            self._cesium_omniverse_interface.addCesiumData(stage_id, "")

            # Cape Canaveral
            self._cesium_omniverse_interface.setGeoreferenceOrigin(-80.53, 28.46, -30.0)

            self._cesium_omniverse_interface.addTilesetIon(
                stage_id,
                1387142,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

        def add_cesium_world_terrain():
            """Adds the standard Cesium World Terrain to the stage."""

            stage_id = omni.usd.get_context().get_stage_id()

            self._cesium_omniverse_interface.addCesiumData(stage_id, "")

            # Cesium HQ
            self._cesium_omniverse_interface.setGeoreferenceOrigin(-75.1564977, 39.9501464, 150.0)

            tileset_id = self._cesium_omniverse_interface.addTilesetIon(
                stage_id,
                1,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

            self._cesium_omniverse_interface.addIonRasterOverlay(
                tileset_id,
                "Layer",
                3954,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

        def add_bing_maps_terrain():
            """Adds the Bing Maps & Cesium Terrain to the stage."""

            stage_id = omni.usd.get_context().get_stage_id()

            self._cesium_omniverse_interface.addCesiumData(stage_id, "")

            # Cesium HQ
            self._cesium_omniverse_interface.setGeoreferenceOrigin(-75.1564977, 39.9501464, 150.0)

            tileset_id = self._cesium_omniverse_interface.addTilesetIon(
                stage_id,
                1,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

            self._cesium_omniverse_interface.addIonRasterOverlay(
                tileset_id,
                "Layer",
                2,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

        async def do_add_bing_maps_using_stage_token():
            await wait_n_frames(1)

            # Cesium HQ
            self._cesium_omniverse_interface.setGeoreferenceOrigin(-75.1564977, 39.9501464, 150.0)

            stage_id = omni.usd.get_context().get_stage_id()
            stage = omni.usd.get_context().get_stage()
            cesium_prim = stage.GetPrimAtPath("/Cesium")
            ion_token = cesium_prim.GetAttribute("ionToken").Get()

            tileset_id = self._cesium_omniverse_interface.addTilesetIon(
                stage_id,
                1,
                ion_token,
            )

            self._cesium_omniverse_interface.addIonRasterOverlay(
                tileset_id,
                "Layer",
                2,
                ion_token,
            )

        def add_bing_maps_terrain_using_stage_token():
            """Adds the Bing Maps & Cesium Terrain to the stage."""

            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.addCesiumData(
                stage_id,
                "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiIyMjRhYzI0Yi1kNWEwLTQ4ZWYtYjdmZC1hY2JmYWIzYmFiMGUiLCJpZCI6NDQsImlhdCI6MTY2NzQ4OTg0N30.du0tvWptgLWsvM1Gnbv3Zw_pDAOILg1Wr6s2sgK-qlM",
            )

            asyncio.ensure_future(do_add_bing_maps_using_stage_token())

        def create_tileset(tileset=Tileset.CESIUM_WORLD_TERRAIN):
            """Creates the desired tileset on the stage.

            Parameters:
                tileset (Tileset): The desired tileset specified using the Tileset enumeration.
            """

            self.destroy_subscription()

            self.update_far_plane()

            # TODO: Eventually we need a real way to do this.
            if tileset is Tileset.CAPE_CANAVERAL:
                add_maxar_3d_surface_model()
            elif tileset is Tileset.BING_MAPS:
                add_bing_maps_terrain()
            elif tileset is Tileset.BING_MAPS_WITH_STAGE_TOKEN:
                add_bing_maps_terrain_using_stage_token()
            else:  # Terrain is Cesium World Terrain
                add_cesium_world_terrain()

        def add_cube_usdrt():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.addCubeUsdrt(stage_id)

        def add_cube_usd():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.addCubeUsd(stage_id)

        def add_cube_fabric():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.addCubeFabric(stage_id)

        def remove_cube_usdrt():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.removeCubeUsdrt(stage_id)

        def show_cube_usdrt():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.showCubeUsdrt(stage_id)

        def hide_cube_usdrt():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.hideCubeUsdrt(stage_id)

        def print_usdrt_stage():
            stage_id = omni.usd.get_context().get_stage_id()
            usdrt_stage = self._cesium_omniverse_interface.printUsdrtStage(stage_id)
            self._cesium_message_field.set_value(usdrt_stage)

        def print_fabric_stage():
            stage_id = omni.usd.get_context().get_stage_id()
            usdrt_stage = self._cesium_omniverse_interface.printFabricStage(stage_id)
            self._cesium_message_field.set_value(usdrt_stage)

        def populate_usd_stage_into_fabric():
            stage_id = omni.usd.get_context().get_stage_id()
            self._cesium_omniverse_interface.populateUsdStageIntoFabric(stage_id)

        def print_fabric_prim_python():
            stage_id = omni.usd.get_context().get_stage_id()
            fabric_info = get_fabric_data_for_prim(stage_id, "/example_material_usd/OmniPBR")
            self._cesium_message_field.set_value(fabric_info)

        with ui.VStack():
            with ui.VStack():
                ui.Button("Update Frame", clicked_fn=lambda: start_update_frame())
                ui.Button("Stop Update Frame", clicked_fn=lambda: stop_update_frame())
                ui.Button(
                    "Create Cesium World Terrain Tileset",
                    clicked_fn=lambda: create_tileset(Tileset.CESIUM_WORLD_TERRAIN),
                )
                ui.Button("Create Bing Maps Tileset", clicked_fn=lambda: create_tileset(Tileset.BING_MAPS))
                ui.Button("Create Cape Canaveral Tileset", clicked_fn=lambda: create_tileset(Tileset.CAPE_CANAVERAL))
                ui.Button("Add Cube USDRT", clicked_fn=lambda: add_cube_usdrt())
                ui.Button("Add Cube USD", clicked_fn=lambda: add_cube_usd())
                ui.Button("Add Cube Fabric", clicked_fn=lambda: add_cube_fabric())
                ui.Button("Remove Cube USDRT", clicked_fn=lambda: remove_cube_usdrt())
                ui.Button("Show Cube USDRT", clicked_fn=lambda: show_cube_usdrt())
                ui.Button("Hide Cube USDRT", clicked_fn=lambda: hide_cube_usdrt())
                ui.Button("Print USDRT stage", clicked_fn=lambda: print_usdrt_stage())
                ui.Button("Print Fabric stage", clicked_fn=lambda: print_fabric_stage())
                ui.Button("Populate USD Stage into Fabric", clicked_fn=lambda: populate_usd_stage_into_fabric())
                ui.Button("Print Fabric prim (python)", clicked_fn=lambda: print_fabric_prim_python())
            with ui.VStack():
                self._cesium_message_field = ui.SimpleStringModel("")
                ui.StringField(self._cesium_message_field, multiline=True, read_only=True)
