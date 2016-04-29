
#
# External dependencies
#

find_package(GLM REQUIRED)
find_package(cpplocate REQUIRED)
find_package(glkernel REQUIRED)
find_package(glbinding REQUIRED)
find_package(globjects REQUIRED)
find_package(libzeug REQUIRED)
find_package(gloperate REQUIRED)
find_package(ASSIMP REQUIRED)


#
# Library name and options
#

# Target name
set(target mfs-painters)

# Exit here if required dependencies are not met
message(STATUS "Lib ${target}")

# Set API export file and macro
string(TOUPPER ${target} target_upper)
set(feature_file "include/${target}/${target}_features.h")
set(export_file  "include/${target}/${target}_api.h")
set(export_macro "${target_upper}_API")


#
# Sources
#

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(source_path  "${CMAKE_CURRENT_SOURCE_DIR}")

set(headers
    ${include_path}/multiframepainter/mfs-painters-api.h

    ${include_path}/multiframepainter/MultiFramePainter.h
    ${include_path}/multiframepainter/ModelLoadingStage.h
    ${include_path}/multiframepainter/KernelGenerationStage.h
    ${include_path}/multiframepainter/RasterizationStage.h
    ${include_path}/multiframepainter/DeferredShadingStage.h
    ${include_path}/multiframepainter/PostprocessingStage.h
    ${include_path}/multiframepainter/FrameAccumulationStage.h
    ${include_path}/multiframepainter/BlitStage.h

    ${include_path}/multiframepainter/TypeDefinitions.h
    ${include_path}/multiframepainter/Preset.h
    ${include_path}/multiframepainter/OmnidirectionalShadowmap.h
    ${include_path}/multiframepainter/Shadowmap.h
    ${include_path}/multiframepainter/Material.h
)

set(sources
    ${source_path}/plugin.cpp

    ${source_path}/multiframepainter/MultiFramePainter.cpp
    ${source_path}/multiframepainter/ModelLoadingStage.cpp
    ${source_path}/multiframepainter/KernelGenerationStage.cpp
    ${source_path}/multiframepainter/RasterizationStage.cpp
    ${source_path}/multiframepainter/PostprocessingStage.cpp
    ${source_path}/multiframepainter/DeferredShadingStage.cpp
    ${source_path}/multiframepainter/FrameAccumulationStage.cpp
    ${source_path}/multiframepainter/BlitStage.cpp

    ${source_path}/multiframepainter/OmnidirectionalShadowmap.cpp
    ${source_path}/multiframepainter/Shadowmap.cpp
    ${source_path}/multiframepainter/Material.cpp
)

# Group source files
set(header_group "Header Files (API)")
set(source_group "Source Files")
source_group_by_path(${include_path} "\\\\.h$|\\\\.hpp$"
    ${header_group} ${headers})
source_group_by_path(${source_path}  "\\\\.cpp$|\\\\.c$|\\\\.h$|\\\\.hpp$"
    ${source_group} ${sources})


#
# Create library
#

# Build library
add_library(${target}
    ${sources}
    ${headers}
)

# Create namespaced alias
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)


# Create API export header
generate_export_header(${target}
    EXPORT_FILE_NAME  ${export_file}
    EXPORT_MACRO_NAME ${export_macro}
)


#
# Project options
#

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


#
# Include directories
#

target_include_directories(${target}
    PRIVATE
    ${PROJECT_BINARY_DIR}/source/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${GLM_INCLUDE_DIR}
    ${ASSIMP_INCLUDE_DIR}

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


#
# Libraries
#

target_link_libraries(${target}
    PRIVATE
    ${ASSIMP_LIBRARIES}
    cpplocate::cpplocate
    libzeug::iozeug
    libzeug::reflectionzeug
    glbinding::glbinding
    globjects::globjects
    gloperate::gloperate
    gloperate::gloperate-assimp
    glkernel::glkernel

    PUBLIC
    ${DEFAULT_LIBRARIES}

    INTERFACE
)


#
# Compile definitions
#

target_compile_definitions(${target}
    PRIVATE

    PUBLIC
    $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target_upper}_STATIC_DEFINE>
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE
)


#
# Compile options
#

target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)


#
# Linker options
#

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)


#
# Deployment (disabled)
#

# Library
install(TARGETS ${target}
    EXPORT  "${target}-export"            COMPONENT dev
    RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
    LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
#    ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# Header files
#install(DIRECTORY
#    ${CMAKE_CURRENT_SOURCE_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
#    COMPONENT dev
#)

# Generated header files
#install(DIRECTORY
#    ${CMAKE_CURRENT_BINARY_DIR}/include/${target} DESTINATION ${INSTALL_INCLUDE}
#    COMPONENT dev
#)

# CMake config
#install(EXPORT ${target}-export
#    NAMESPACE   ${META_PROJECT_NAME}::
#    DESTINATION ${INSTALL_CMAKE}/${target}
#    COMPONENT   dev
#)
