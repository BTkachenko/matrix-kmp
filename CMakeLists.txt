cmake_minimum_required(VERSION 3.15)
project(matrix_native LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Release")
  add_compile_options(-O3 -DNDEBUG -march=native)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_link_options(-flto)
  endif()
endif()

add_library(matrix SHARED
    native/src/matrix_core.cpp
    native/src/matrix_c.cpp)

target_include_directories(matrix PUBLIC native/include)

set_target_properties(matrix PROPERTIES
    OUTPUT_NAME "matrix"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

find_package(JNI REQUIRED)
add_library(matrix_jni SHARED native/jni/matrix_jni.cpp)
target_include_directories(matrix_jni PRIVATE ${JNI_INCLUDE_DIRS} native/include)
target_link_libraries(matrix_jni PRIVATE matrix ${JNI_LIBRARIES})

set_target_properties(matrix_jni PROPERTIES
    OUTPUT_NAME "matrix_jni"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
