cmake_minimum_required(VERSION 3.11)
project(Swap VERSION 0.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_definitions(${PROJECT_NAME} PUBLIC SWAP_DEBUG=$<CONFIG:Debug>)
target_compile_definitions(${PROJECT_NAME} PUBLIC SWAP_PLATFORM_WINDOWS=$<PLATFORM_ID:Windows>)
target_compile_definitions(${PROJECT_NAME} PUBLIC SWAP_PLATFORM_MACOS=$<PLATFORM_ID:Darwin>)
target_compile_definitions(${PROJECT_NAME} PUBLIC SWAP_PLATFORM_LINUX=$<PLATFORM_ID:Linux>)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/Source")
set(RES_DIR "${PROJECT_SOURCE_DIR}/Resources")
set(LIB_DIR "${PROJECT_SOURCE_DIR}/Libraries")

include("${PROJECT_SOURCE_DIR}/Source.cmake")
include("${PROJECT_SOURCE_DIR}/Resources.cmake")
include("${PROJECT_SOURCE_DIR}/Libraries.cmake")
