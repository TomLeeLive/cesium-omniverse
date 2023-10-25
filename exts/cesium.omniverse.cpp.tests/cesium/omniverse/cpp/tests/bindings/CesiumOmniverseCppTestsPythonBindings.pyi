class ICesiumOmniverseCppTestsInterface:
    def __init__(self, *args, **kwargs) -> None: ...
    def on_shutdown(self) -> None: ...
    def on_startup(self, arg0: str) -> None: ...
    def run_all_tests(self) -> None: ...
    def set_up_tests(self, arg0: int) -> None: ...

def acquire_cesium_omniverse_tests_interface(
    plugin_name: str = ..., library_path: str = ...
) -> ICesiumOmniverseCppTestsInterface: ...
def release_cesium_omniverse_tests_interface(arg0: ICesiumOmniverseCppTestsInterface) -> None: ...
