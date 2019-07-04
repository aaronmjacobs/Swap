cmake_minimum_required(VERSION 3.11)
project(glad VERSION 0.1.24 LANGUAGES C)
set(DEBUG_PROJECT_NAME "${PROJECT_NAME}-debug")

add_library(${PROJECT_NAME} "")
add_library(${DEBUG_PROJECT_NAME} "")

set(INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/include")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/release/src")
set(DEBUG_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/debug/include")
set(DEBUG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/debug/src")

target_sources(${PROJECT_NAME} PRIVATE
   "${INC_DIR}/glad/gl.h"
   "${INC_DIR}/KHR/khrplatform.h"
   "${SRC_DIR}/gl.c"
)
target_sources(${DEBUG_PROJECT_NAME} PRIVATE
   "${DEBUG_INC_DIR}/glad/gl.h"
   "${DEBUG_INC_DIR}/KHR/khrplatform.h"
   "${DEBUG_SRC_DIR}/gl.c"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${INC_DIR}")
target_include_directories(${DEBUG_PROJECT_NAME} PUBLIC "${DEBUG_INC_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})
target_link_libraries(${DEBUG_PROJECT_NAME} PUBLIC ${CMAKE_DL_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "${PROJECT_NAME}")
set_target_properties(${DEBUG_PROJECT_NAME} PROPERTIES FOLDER "${PROJECT_NAME}")
