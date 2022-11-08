project(vtkCjyx${MODULE_NAME}ModuleDMML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_CJYX_${MODULE_NAME_UPPER}_MODULE_DMML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${DMMLCore_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkDMML${MODULE_NAME}Node.cxx
  vtkDMML${MODULE_NAME}Node.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${DMML_LIBRARIES}
  vtkCjyxMarkupsModuleDMML
  )

# --------------------------------------------------------------------------
CjyxMacroBuildModuleDMML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
