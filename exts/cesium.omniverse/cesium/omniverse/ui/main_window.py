from ..bindings import ICesiumOmniverseInterface, CesiumIonSession
import logging
import carb.events
import omni.kit.app as app
import omni.ui as ui
import webbrowser
from pathlib import Path
from typing import List, Optional
from .sign_in_widget import CesiumOmniverseSignInWidget
from .styles import CesiumOmniverseUiStyles

HELP_URL = "https://community.cesium.com/"
LEARN_URL = "https://cesium.com/learn/"
UPLOAD_URL = "https://ion.cesium.com/addasset"


class CesiumOmniverseMainWindow(ui.Window):
    """
    The main window for working with Cesium for Omniverse. Docked in the same area as "Stage".
    """

    WINDOW_NAME = "Cesium"
    MENU_PATH = f"Window/Cesium/{WINDOW_NAME}"

    def __init__(self, cesium_omniverse_interface: ICesiumOmniverseInterface, **kwargs):
        super().__init__(CesiumOmniverseMainWindow.WINDOW_NAME, **kwargs)
        manager = app.get_app().get_extension_manager()
        ext_id = manager.get_extension_id_by_module("cesium.omniverse")

        self._cesium_omniverse_interface = cesium_omniverse_interface
        self._logger = logging.getLogger(__name__)
        self._icon_path = Path(manager.get_extension_path(ext_id)).joinpath("images")

        # Buttons aren't created until the build function is called.
        self._add_button: Optional[ui.Button] = None
        self._upload_button: Optional[ui.Button] = None
        self._token_button: Optional[ui.Button] = None
        self._learn_button: Optional[ui.Button] = None
        self._help_button: Optional[ui.Button] = None
        self._sign_out_button: Optional[ui.Button] = None
        self._sign_in_widget: Optional[CesiumOmniverseSignInWidget] = None

        self._subscriptions: List[carb.events.ISubscription] = []
        self._setup_subscriptions()

        self.frame.set_build_fn(self._build_fn)

    def destroy(self) -> None:
        for subscription in self._subscriptions:
            subscription.unsubscribe()

        if self._sign_in_widget:
            self._sign_in_widget.destroy()

        super().destroy()

    def _setup_subscriptions(self):
        update_stream = app.get_app().get_update_event_stream()
        bus = app.get_app().get_message_bus_event_stream()

        self._subscriptions.append(
            update_stream.create_subscription_to_pop(self._on_update_frame, name="on_update_frame"))

        assets_updated_event = carb.events.type_from_string("cesium.omniverse.ASSETS_UPDATED")
        self._subscriptions.append(
            bus.create_subscription_to_pop_by_type(assets_updated_event, self._on_assets_updated, name="assets_updated")
        )

        connection_updated_event = carb.events.type_from_string("cesium.omniverse.CONNECTION_UPDATED")
        self._subscriptions.append(
            bus.create_subscription_to_pop_by_type(connection_updated_event, self._on_connection_updated,
                                                   name="connection_updated")
        )

        profile_updated_event = carb.events.type_from_string("cesium.omniverse.PROFILE_UPDATED")
        self._subscriptions.append(
            bus.create_subscription_to_pop_by_type(profile_updated_event, self._on_profile_updated,
                                                   name="profile_updated")
        )

        tokens_updated_event = carb.events.type_from_string("cesium.omniverse.TOKENS_UPDATED")
        self._subscriptions.append(
            bus.create_subscription_to_pop_by_type(tokens_updated_event, self._on_tokens_updated, name="tokens_updated")
        )

    def _on_update_frame(self, _e: carb.events.IEvent):
        self._cesium_omniverse_interface.on_ui_update()

        session: CesiumIonSession = self._cesium_omniverse_interface.get_session()

        if session is not None and self._sign_in_widget is not None:
            is_connected = session.is_connected()  # Since this goes across the pybind barrier, just grab it once.
            self._sign_in_widget.visible = not is_connected
            self._add_button.enabled = is_connected
            self._upload_button.enabled = is_connected
            self._sign_out_button.enabled = is_connected

    def _on_assets_updated(self, _e: carb.events.IEvent):
        pass

    def _on_connection_updated(self, _e: carb.events.IEvent):
        pass

    def _on_profile_updated(self, _e: carb.events.IEvent):
        pass

    def _on_tokens_updated(self, _e: carb.events.IEvent):
        pass

    def _build_fn(self):
        """Builds all UI components."""

        with ui.VStack(spacing=20):
            button_style = CesiumOmniverseUiStyles.top_bar_button_style

            with ui.HStack(height=ui.Length(80, ui.UnitType.PIXEL)):
                self._add_button = ui.Button("Add", image_url=f"{self._icon_path}/FontAwesome/plus-solid.png",
                                             style=button_style, clicked_fn=self._add_button_clicked, enabled=False)
                self._upload_button = ui.Button("Upload",
                                                image_url=f"{self._icon_path}/FontAwesome/cloud-upload-alt-solid.png",
                                                style=button_style, clicked_fn=self._upload_button_clicked,
                                                enabled=False)
                self._token_button = ui.Button("Token", image_url=f"{self._icon_path}/FontAwesome/key-solid.png",
                                               style=button_style, clicked_fn=self._token_button_clicked)
                self._learn_button = ui.Button("Learn",
                                               image_url=f"{self._icon_path}/FontAwesome/book-reader-solid.png",
                                               style=button_style, clicked_fn=self._learn_button_clicked)
                self._help_button = ui.Button("Help",
                                              image_url=f"{self._icon_path}/FontAwesome/hands-helping-solid.png",
                                              style=button_style, clicked_fn=self._help_button_clicked)
                self._sign_out_button = ui.Button("Sign Out",
                                                  image_url=f"{self._icon_path}/FontAwesome/sign-out-alt-solid.png",
                                                  style=button_style, clicked_fn=self._sign_out_button_clicked,
                                                  enabled=False)
            self._sign_in_widget = CesiumOmniverseSignInWidget(self._cesium_omniverse_interface, visible=False)

    def _add_button_clicked(self) -> None:
        if not self._add_button or not self._add_button.enabled:
            return

        # TODO: Implement CesiumMainWindow._add_button_clicked(self)

        pass

    def _upload_button_clicked(self) -> None:
        if not self._upload_button or not self._upload_button.enabled:
            return

        webbrowser.open(UPLOAD_URL)

    def _token_button_clicked(self) -> None:
        if not self._token_button:
            return

        # TODO: Implement CesiumMainWindow._token_button_clicked(self)

        pass

    def _learn_button_clicked(self) -> None:
        if not self._learn_button:
            return

        webbrowser.open(LEARN_URL)

    def _help_button_clicked(self) -> None:
        if not self._help_button:
            return

        webbrowser.open(HELP_URL)

    def _sign_out_button_clicked(self) -> None:
        if not self._sign_out_button or not self._sign_out_button.enabled:
            return

        session = self._cesium_omniverse_interface.get_session()
        if session is not None:
            session.disconnect()
