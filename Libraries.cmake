set(BUILD_SHARED_LIBS ON CACHE INTERNAL "Build package with shared libraries.")

# Assimp
set(ASSIMP_DIR "${LIB_DIR}/assimp")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "If the supplementary tools for Assimp are built in addition to the library.")
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "If the test suite for Assimp is built in addition to the library.")
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "Disable Assimp's export functionality.")
add_subdirectory("${ASSIMP_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC assimp)
get_property(ASSIMP_INCLUDE_DIRS DIRECTORY "${ASSIMP_DIR}" PROPERTY INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} PUBLIC ${ASSIMP_INCLUDE_DIRS})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:assimp>" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build package with shared libraries.")

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
target_link_libraries(${PROJECT_NAME} PUBLIC glm)

# GSL
add_subdirectory("${LIB_DIR}/GSL")
target_link_libraries(${PROJECT_NAME} PUBLIC GSL)

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
