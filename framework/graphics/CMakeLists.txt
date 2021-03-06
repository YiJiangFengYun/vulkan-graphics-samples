option(VG_ENABLE_VALIDATION_LAYERS "Enable features of vulkan validation layers." ON)
option(VG_ENABLE_COST_TIMER "Enable cost timer for the important step." OFF)

set(LIBRARY_NAME "graphics")

set(GEN_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/src")
set(GEN_SRC_DIR "${GEN_ROOT_DIR}/${LIBRARY_NAME}")

#Pre depth default shader.
compile_shader("${CMAKE_CURRENT_SOURCE_DIR}/shader" "pre_depth_default")
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/pre_depth_vert_default_code.c"
  COMMAND "embed_file_to_code" "VG_PRE_DEPTH_VERT_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/pre_depth_default.vert.spv" "${GEN_SRC_DIR}/pass/pre_depth_vert_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/pre_depth_default.vert.spv"
  DEPENDS "embed_file_to_code"
)
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/pre_depth_frag_default_code.c"
  COMMAND "embed_file_to_code" "VG_PRE_DEPTH_FRAG_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/pre_depth_default.frag.spv" "${GEN_SRC_DIR}/pass/pre_depth_frag_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/pre_depth_default.frag.spv"
  DEPENDS "embed_file_to_code"
)

#Light depth default shader.
compile_shader("${CMAKE_CURRENT_SOURCE_DIR}/shader" "lighting_depth_default")
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/lighting_depth_vert_default_code.c"
  COMMAND "embed_file_to_code" "VG_LIGHTING_DEPTH_VERT_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_depth_default.vert.spv" "${GEN_SRC_DIR}/pass/lighting_depth_vert_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_depth_default.vert.spv"
  DEPENDS "embed_file_to_code"
)
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/lighting_depth_frag_default_code.c"
  COMMAND "embed_file_to_code" "VG_LIGHTING_DEPTH_FRAG_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_depth_default.frag.spv" "${GEN_SRC_DIR}/pass/lighting_depth_frag_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_depth_default.frag.spv"
  DEPENDS "embed_file_to_code"
)

#Light point distance default shader.
compile_shader("${CMAKE_CURRENT_SOURCE_DIR}/shader" "lighting_point_dist_default")
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/lighting_point_dist_vert_default_code.c"
  COMMAND "embed_file_to_code" "VG_LIGHTING_POINT_DIST_VERT_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.vert.spv" "${GEN_SRC_DIR}/pass/lighting_point_dist_vert_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.vert.spv"
  DEPENDS "embed_file_to_code"
)
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/lighting_point_dist_geom_default_code.c"
  COMMAND "embed_file_to_code" "VG_LIGHTING_POINT_DIST_GEOM_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.geom.spv" "${GEN_SRC_DIR}/pass/lighting_point_dist_geom_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.geom.spv"
  DEPENDS "embed_file_to_code"
)
add_custom_command (
  OUTPUT "${GEN_SRC_DIR}/pass/lighting_point_dist_frag_default_code.c"
  COMMAND "embed_file_to_code" "VG_LIGHTING_POINT_DIST_FRAG_DEFAULT_CODE" "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.frag.spv" "${GEN_SRC_DIR}/pass/lighting_point_dist_frag_default_code.c"
  MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/shader/lighting_point_dist_default.frag.spv"
  DEPENDS "embed_file_to_code"
)


configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/config/config.hpp.in"
    "${GEN_SRC_DIR}/config.hpp"
)
set(INCLUDE_DIRS ${INCLUDE_DIRS} "${CMAKE_CURRENT_SOURCE_DIR}")
set(INCLUDE_DIRS ${INCLUDE_DIRS} "${GEN_ROOT_DIR}")
file(GLOB_RECURSE HEADERS "${LIBRARY_NAME}/*.hpp" "${LIBRARY_NAME}/*.inl")
file(GLOB_RECURSE SOURCES "${LIBRARY_NAME}/*.cpp" "${LIBRARY_NAME}/*.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/pre_depth_vert_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/pre_depth_frag_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/lighting_depth_vert_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/lighting_depth_frag_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/lighting_point_dist_vert_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/lighting_point_dist_geom_default_code.c")
set(SOURCES ${SOURCES} "${GEN_SRC_DIR}/pass/lighting_point_dist_frag_default_code.c")
include_directories(${INCLUDE_DIRS})
add_library(${LIBRARY_NAME} STATIC ${HEADERS} ${SOURCES})
set_property(TARGET ${LIBRARY_NAME} PROPERTY FOLDER ${FOLDER_NAME})
set(LIBRARIES ${LIBRARIES} ${LIBRARY_NAME})

# install
install (TARGETS ${LIBRARY_NAME} DESTINATION bin)
install (FILES ${HEADERS} DESTINATION include)

set(INCLUDE_DIRS ${INCLUDE_DIRS} PARENT_SCOPE)
set(LIBRARIES ${LIBRARIES} PARENT_SCOPE)
