cmake_minimum_required(VERSION 3.15)
project(matrix_native LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Core library (C ABI + C++ core)
add_library(matrix SHARED
native/src/matrix_core.cpp
native/src/matrix_c.cpp)


target_include_directories(matrix PUBLIC native/include)


# JNI shim
find_package(JNI REQUIRED)
add_library(matrix_jni SHARED native/jni/matrix_jni.cpp)


target_include_directories(matrix_jni PRIVATE ${JNI_INCLUDE_DIRS} native/include)


target_link_libraries(matrix_jni PRIVATE matrix ${JNI_LIBRARIES})


# On macOS, produce lib names expected by System.loadLibrary("matrix_jni")
set_target_properties(matrix PROPERTIES OUTPUT_NAME "matrix")
set_target_properties(matrix_jni PROPERTIES OUTPUT_NAME "matrix_jni")