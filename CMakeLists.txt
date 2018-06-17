cmake_minimum_required(VERSION 3.11)
project(Swap VERSION 0.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
target_compile_definitions(${PROJECT_NAME} PUBLIC SWAP_DEBUG=$<CONFIG:Debug>)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/Source")
set(LIB_DIR "${PROJECT_SOURCE_DIR}/Libraries")

include("${PROJECT_SOURCE_DIR}/Source.cmake")
include("${PROJECT_SOURCE_DIR}/Libraries.cmake")
