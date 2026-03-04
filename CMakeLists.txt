cmake_minimum_required(VERSION 3.10)

project(CVDevCommunityPlugins)

set(GFLAGS_USE_TARGET_NAMESPACE ON)

add_subdirectory(GPLNodes)
add_subdirectory(YoloOnnxNodes)
