from ..bindings import ICesiumOmniverseInterface
import logging
import carb.events
import omni.kit.app as app
import omni.ui as ui
from typing import List


class IonAssetItem(ui.AbstractItem):
    """Represents an ion Asset."""

    def __init__(self, asset_id: int, name: str, description: str, attribution: str, asset_type: str, date_added: str):
        super().__init__()
        self.id = ui.SimpleIntModel(asset_id)
        self.name = ui.SimpleStringModel(name)
        self.description = ui.SimpleStringModel(description)
        self.attribution = ui.SimpleStringModel(attribution)
        self.type = ui.SimpleStringModel(asset_type)
        self.dateAdded = ui.SimpleStringModel(date_added)

    def __repr__(self):
        return f"{self.name.as_string} (ID: {self.id.as_int})"


class IonAssets(ui.AbstractItemModel):
    """Represents a list of ion assets for the asset window."""

    def __init__(self, items: List[IonAssetItem] = []):
        super().__init__()
        self._items: List[IonAssetItem] = items

    def replace_items(self, items: List[IonAssetItem]):
        self._items.clear()
        self._items.extend(items)
        self._item_changed(None)

    def get_item_children(self, item: IonAssetItem = None) -> List[IonAssetItem]:
        if item is not None:
            return []

        return self._items

    def get_item_value_model_count(self, item: IonAssetItem = None) -> int:
        """The number of columns"""
        return 3

    def get_item_value_model(self, item: IonAssetItem = None, column_id: int = 0) -> ui.AbstractValueModel:
        """Returns the value model for the specific column."""

        if item is None:
            item = self._items[0]

        # When we are finally on Python 3.10 with Omniverse, we should change this to a switch.
        return item.name if column_id == 0 else item.type if column_id == 1 else item.dateAdded


class IonAssetDelegate(ui.AbstractItemDelegate):

    def build_header(self, column_id: int = 0) -> None:
        stack = ui.ZStack(height=20)
        with stack:
            if column_id == 0:
                ui.Label("Name")
            elif column_id == 1:
                ui.Label("Type")
            else:
                ui.Label("Date Added")

    def build_branch(self, model: ui.AbstractItemModel, item: ui.AbstractItem = None, column_id: int = 0,
                     level: int = 0,
                     expanded: bool = False) -> None:
        pass

    def build_widget(self, model: IonAssets, item: IonAssetItem = None, index: int = 0, level: int = 0,
                     expanded: bool = False) -> None:
        pass


class CesiumOmniverseAssetWindow(ui.Window):
    """
    The asset list window for Cesium for Omniverse. Docked in the same area as "Assets".
    """

    WINDOW_NAME = "Cesium Assets"
    MENU_PATH = f"Window/Cesium/{WINDOW_NAME}"

    def __init__(self, cesium_omniverse_interface: ICesiumOmniverseInterface, **kwargs):
        super().__init__(CesiumOmniverseAssetWindow.WINDOW_NAME, **kwargs)

        self._cesium_omniverse_interface = cesium_omniverse_interface
        self._logger = logging.getLogger(__name__)

        self._assets = IonAssets()
        self._assets_delegate = IonAssetDelegate()

        self._subscriptions: List[carb.events.ISubscription] = []

        self.frame.set_build_fn(self._build_fn)

    def destroy(self):
        for subscription in self._subscriptions:
            subscription.unsubscribe()
        self._subscriptions.clear()

        super().destroy()

    def _setup_subscriptions(self):
        bus = app.get_app().get_message_bus_event_stream()

        assets_updated_event = carb.events.type_from_string("cesium.omniverse.ASSETS_UPDATED")
        self._subscriptions.append(
            bus.create_subscription_to_pop_by_type(assets_updated_event, self._on_assets_updated,
                                                   name="cesium.omniverse.asset_window.assets_updated")
        )

    def _refresh_list(self):
        pass

    def _on_assets_updated(self):
        pass

    def _build_fn(self):
        """Builds all UI components."""

        with ui.VStack():
            # TODO: Top bar with refresh button.
            with ui.ScrollingFrame(style_type_name_override="TreeView",
                                   style={"Field": {"background_color": 0xFF000000}}):
                ui.TreeView(self._assets, delegate=self._assets_delegate, root_visible=False, header_visible=True,
                            style={"TreeView.Item": {"margin": 4}})
