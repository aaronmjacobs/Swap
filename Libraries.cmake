# Boxer
add_subdirectory("${LIB_DIR}/Boxer")
target_link_libraries(${PROJECT_NAME} PUBLIC Boxer)

# glad
add_subdirectory("${LIB_DIR}/glad")
target_link_libraries(${PROJECT_NAME} PUBLIC optimized glad)
target_link_libraries(${PROJECT_NAME} PUBLIC debug glad-debug)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${LIB_DIR}/glfw")
target_compile_definitions(${PROJECT_NAME} PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

# GLM
set(GLM_INSTALL_ENABLE OFF CACHE INTERNAL "GLM install")
add_subdirectory("${LIB_DIR}/glm")

# PPK_ASSERT
set(PPK_DIR "${LIB_DIR}/PPK_ASSERT")
target_sources(${PROJECT_NAME} PRIVATE "${PPK_DIR}/src/ppk_assert.h" "${PPK_DIR}/src/ppk_assert.cpp")
target_include_directories(${PROJECT_NAME} PUBLIC "${PPK_DIR}/src")
source_group("Libraries\\PPK_ASSERT" "${PPK_DIR}/src")

# stb
set(STB_DIR "${LIB_DIR}/stb")
target_sources(${PROJECT_NAME} PRIVATE
   "${STB_DIR}/stb.h"
   "${STB_DIR}/stb_c_lexer.h"
   "${STB_DIR}/stb_connected_components.h"
   "${STB_DIR}/stb_divide.h"
   "${STB_DIR}/stb_dxt.h"
   "${STB_DIR}/stb_easy_font.h"
   "${STB_DIR}/stb_herringbone_wang_tile.h"
   "${STB_DIR}/stb_image.h"
   "${STB_DIR}/stb_image_resize.h"
   "${STB_DIR}/stb_image_write.h"
   "${STB_DIR}/stb_leakcheck.h"
   "${STB_DIR}/stb_perlin.h"
   "${STB_DIR}/stb_rect_pack.h"
   "${STB_DIR}/stb_sprintf.h"
   "${STB_DIR}/stb_textedit.h"
   "${STB_DIR}/stb_tilemap_editor.h"
   "${STB_DIR}/stb_truetype.h"
#  "${STB_DIR}/stb_vorbis.c"
   "${STB_DIR}/stb_voxel_render.h"
   "${STB_DIR}/stretchy_buffer.h"
)
target_include_directories(${PROJECT_NAME} PUBLIC "${STB_DIR}")
source_group("Libraries\\stb" "${STB_DIR}")

# templog
set(TEMPLOG_DIR "${LIB_DIR}/templog")
target_sources(${PROJECT_NAME} PRIVATE
   "${TEMPLOG_DIR}/config.h"
   "${TEMPLOG_DIR}/logging.h"
   "${TEMPLOG_DIR}/templ_meta.h"
   "${TEMPLOG_DIR}/tuples.h"
   "${TEMPLOG_DIR}/type_lists.h"
   "${TEMPLOG_DIR}/imp/logging.cpp"
)
target_include_directories(${PROJECT_NAME} PUBLIC "${TEMPLOG_DIR}")
source_group("Libraries\\templog" "${TEMPLOG_DIR}")

# tinyobj
add_subdirectory("${LIB_DIR}/tinyobjloader")
target_link_libraries(${PROJECT_NAME} PUBLIC tinyobjloader)
