cmake_minimum_required(VERSION 3.8)
project(Swap VERSION 0.0.0 LANGUAGES CXX)

add_executable(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/Source/Main.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Source")
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_14)
