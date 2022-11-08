project(vtkCjyx${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

add_subdirectory(FastMarching)

#
# vtkAddon
#
if(NOT Cjyx_BUILD_vtkAddon)
  find_package(vtkAddon REQUIRED)
endif()

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkAddon_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/gw_geodesic
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/gw_core
  )

set(${KIT}_SRCS
  vtkImplicitPolyDataPointDistance.cxx
  vtkImplicitPolyDataPointDistance.h
  vtkCjyx${MODULE_NAME}AppendTool.cxx
  vtkCjyx${MODULE_NAME}AppendTool.h
  vtkCjyx${MODULE_NAME}BoundaryCutTool.cxx
  vtkCjyx${MODULE_NAME}BoundaryCutTool.h
  vtkCjyx${MODULE_NAME}CurveCutTool.cxx
  vtkCjyx${MODULE_NAME}CurveCutTool.h
  vtkCjyx${MODULE_NAME}HollowTool.cxx
  vtkCjyx${MODULE_NAME}HollowTool.h
  vtkCjyx${MODULE_NAME}Logic.cxx
  vtkCjyx${MODULE_NAME}Logic.h
  vtkCjyx${MODULE_NAME}MarginTool.cxx
  vtkCjyx${MODULE_NAME}MarginTool.h
  vtkCjyx${MODULE_NAME}MirrorTool.cxx
  vtkCjyx${MODULE_NAME}MirrorTool.h
  vtkCjyx${MODULE_NAME}PlaneCutTool.cxx
  vtkCjyx${MODULE_NAME}PlaneCutTool.h
  vtkCjyx${MODULE_NAME}ROICutTool.cxx
  vtkCjyx${MODULE_NAME}ROICutTool.h
  vtkCjyx${MODULE_NAME}SelectByPointsTool.cxx
  vtkCjyx${MODULE_NAME}SelectByPointsTool.h
  vtkCjyx${MODULE_NAME}Tool.cxx
  vtkCjyx${MODULE_NAME}Tool.h
  vtkCjyx${MODULE_NAME}ToolFactory.cxx
  vtkCjyx${MODULE_NAME}ToolFactory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkFastMarchingGeodesicDistance.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkFastMarchingGeodesicDistance.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkFastMarchingGeodesicPath.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkFastMarchingGeodesicPath.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkPolyDataGeodesicDistance.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkPolyDataGeodesicDistance.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkPolygonalSurfaceContourLineInterpolator2.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/FastMarching/vtkPolygonalSurfaceContourLineInterpolator2.h
  )

set(${KIT}_TARGET_LIBRARIES
  vtkCjyx${MODULE_NAME}ModuleDMML
  vtkAddon
  MeshGeodesics
  )

#-----------------------------------------------------------------------------
CjyxMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
