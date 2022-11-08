cmake_minimum_required(VERSION 3.13.4)

project(SurfaceToolbox)

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Cjyx REQUIRED)
include(${Cjyx_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(Decimation)
add_subdirectory(SurfaceToolbox)
add_subdirectory(DynamicModeler)

## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Cjyx_EXTENSION_GENERATE_CONFIG})
include(${Cjyx_EXTENSION_CPACK})
