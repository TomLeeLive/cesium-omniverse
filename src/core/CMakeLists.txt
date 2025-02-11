# On Linux you may see a warning like:
#
# Warning: Unused direct dependencies:
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libarch.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libjs.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libkind.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libndr.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libpcp.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libplug.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libsdr.so
#         cesium-omniverse/extern/nvidia/_build/target-deps/usd/release/lib/libtrace.so
#
# This is because some USD libraries are referenced by other USD libraries but are not direct depenencies of
# Cesium Omniverse. The reason we include them in the list below is because on Windows this allows them to be
# copied to the bin folder when TARGET_RUNTIME_DLLS is used.

include(Macros)

glob_files(SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

get_property(ADDITIONAL_LIBRARIES GLOBAL PROPERTY NVIDIA_ADDITIONAL_LIBRARIES_PROPERTY)
get_property(ADDITIONAL_LINK_DIRECTORIES GLOBAL PROPERTY NVIDIA_ADDITIONAL_LINK_DIRECTORIES_PROPERTY)

# cmake-format: off
setup_lib(
    TARGET_NAME
        CesiumOmniverseCore
    TYPE
        # CesiumOmniverseCore needs to be built as a STATIC library always so that it can be tested.
        # Otherwise if it was built as a shared library we would have to add dllexport attributes to almost everything.
        STATIC
    SOURCES
        ${SOURCES}
    INCLUDE_DIRS
        "${CMAKE_CURRENT_LIST_DIR}/include"
    LIBRARIES
        CesiumUsdSchemas
        Cesium3DTilesSelection
        Cesium3DTilesReader
        Cesium3DTilesContent
        CesiumRasterOverlays
        CesiumGltfReader
        CesiumGltfContent
        CesiumGltf
        CesiumJsonReader
        CesiumGeospatial
        CesiumGeometry
        CesiumIonClient
        CesiumAsync
        CesiumUtility
        async++
        csprng
        draco
        ktx_read
        modp_b64
        s2geometry
        spdlog
        tinyxml2
        uriparser
        webpdecoder
        turbojpeg
        meshoptimizer
        cpr::cpr
        stb::stb
        ZLIB::ZLIB
        fabric
        ar
        arch
        gf
        js
        kind
        ndr
        pcp
        plug
        sdf
        sdr
        tf
        trace
        usd
        usdGeom
        usdLux
        usdShade
        usdUI
        usdUtils
        usdVol
        vt
        work
        boost_python310
        tbb
        carb
        omni_kit
        omni_ui
        cuda
        pybind11
        python310
    ADDITIONAL_LIBRARIES
        "${ADDITIONAL_LIBRARIES}"
    ADDITIONAL_LINK_DIRECTORIES
        "${ADDITIONAL_LINK_DIRECTORIES}"
    DEPENDENCIES
        cesium-native-external
    CXX_FLAGS
        ${CESIUM_OMNI_CXX_FLAGS}
    CXX_FLAGS_DEBUG
        ${CESIUM_OMNI_CXX_FLAGS_DEBUG}
    CXX_DEFINES
        ${CESIUM_OMNI_CXX_DEFINES}
    CXX_DEFINES_DEBUG
        ${CESIUM_OMNI_CXX_DEFINES_DEBUG}
)
# cmake-format: on
