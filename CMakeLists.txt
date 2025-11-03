cmake_minimum_required(VERSION 3.16)
project(matrix_native LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_OK OUTPUT IPO_MSG)

add_library(matrix SHARED
  native/src/matrix_core.cpp
  native/src/matrix_c.cpp
)
target_include_directories(matrix PUBLIC native/include)

if(CMAKE_BUILD_TYPE MATCHES "Release")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(matrix PRIVATE -O3 -DNDEBUG -march=native -ffast-math -fno-math-errno -funroll-loops)
  elseif(MSVC)
    target_compile_options(matrix PRIVATE /O2 /fp:fast /GL)
  endif()
  if(IPO_OK)
    set_property(TARGET matrix PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

find_package(JNI REQUIRED)
add_library(matrix_jni SHARED native/jni/matrix_jni.cpp)
target_include_directories(matrix_jni PRIVATE ${JNI_INCLUDE_DIRS} native/include)
target_link_libraries(matrix_jni PRIVATE matrix ${JNI_LIBRARIES})

if(CMAKE_BUILD_TYPE MATCHES "Release")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(matrix_jni PRIVATE -O3 -DNDEBUG -march=native -ffast-math -fno-math-errno -funroll-loops)
  elseif(MSVC)
    target_compile_options(matrix_jni PRIVATE /O2 /fp:fast /GL)
  endif()
  if(IPO_OK)
    set_property(TARGET matrix_jni PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endif()

set_target_properties(matrix PROPERTIES
  OUTPUT_NAME "matrix"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
set_target_properties(matrix_jni PROPERTIES
  OUTPUT_NAME "matrix_jni"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

option(MX_USE_OPENMP "Enable OpenMP parallelization" ON)
if(MX_USE_OPENMP)
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
    target_link_libraries(matrix PRIVATE OpenMP::OpenMP_CXX)
  endif()
endif()
