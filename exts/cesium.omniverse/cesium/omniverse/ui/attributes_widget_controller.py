import logging
import omni.kit.window.property
from .attributes import (
    CesiumDataSchemaAttributesWidget,
    CesiumGeoreferenceSchemaAttributesWidget,
    CesiumImageryAttributesWidget,
    CesiumTilesetAttributesWidget,
    CesiumGlobeAnchorAttributesWidget,
    CesiumIonServerAttributesWidget,
)
from ..bindings import ICesiumOmniverseInterface


class CesiumAttributesWidgetController:
    """
    This is designed as a helpful function for separating out the registration and
    unregistration of Cesium's attributes widgets.
    """

    def __init__(self, _cesium_omniverse_interface: ICesiumOmniverseInterface):
        self._cesium_omniverse_interface = _cesium_omniverse_interface
        self._logger = logging.getLogger(__name__)

        self._register_data_attributes_widget()
        self._register_georeference_attributes_widget()
        self._register_tileset_attributes_widget()
        self._register_imagery_attributes_widget()
        self._register_global_anchor_attributes_widget()
        self._register_ion_server_attributes_widget()

    def destroy(self):
        self._unregister_data_attributes_widget()
        self._unregister_georeference_attributes_widget()
        self._unregister_tileset_attributes_widget()
        self._unregister_imagery_attributes_widget()
        self._unregister_global_anchor_attributes_widget()
        self._unregister_ion_server_attributes_widget()

    @staticmethod
    def _register_data_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget("prim", "cesiumData", CesiumDataSchemaAttributesWidget())

    @staticmethod
    def _unregister_data_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumData")

    @staticmethod
    def _register_georeference_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget("prim", "cesiumGeoreference", CesiumGeoreferenceSchemaAttributesWidget())

    @staticmethod
    def _unregister_georeference_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumGeoreference")

    def _register_tileset_attributes_widget(self):
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget(
                "prim", "cesiumTileset", CesiumTilesetAttributesWidget(self._cesium_omniverse_interface)
            )

    @staticmethod
    def _unregister_tileset_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumTileset")

    @staticmethod
    def _register_imagery_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget("prim", "cesiumImagery", CesiumImageryAttributesWidget())

    @staticmethod
    def _unregister_imagery_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumImagery")

    def _register_global_anchor_attributes_widget(self):
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget(
                "prim", "cesiumGlobeAnchorAPI", CesiumGlobeAnchorAttributesWidget(self._cesium_omniverse_interface)
            )

    @staticmethod
    def _unregister_global_anchor_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumGlobalAnchorAPI")

    def _register_ion_server_attributes_widget(self):
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.register_widget(
                "prim", "cesiumIonServer", CesiumIonServerAttributesWidget(self._cesium_omniverse_interface)
            )

    @staticmethod
    def _unregister_ion_server_attributes_widget():
        window = omni.kit.window.property.get_window()
        if window is not None:
            window.unregister_widget("prim", "cesiumIonServer")
