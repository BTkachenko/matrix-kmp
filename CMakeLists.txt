cmake_minimum_required(VERSION 3.15)
project(matrix_native LANGUAGES C CXX)

# ---- Toolchain / language ----------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # PIC for all targets (safe on Linux)

# ---- Uniform output dirs: ${build}/lib for all kinds of artifacts -----------
# (Also set per-target below to be explicit across generators.)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ---- Core library: C++ implementation + C ABI layer -------------------------
add_library(matrix SHARED
    native/src/matrix_core.cpp
    native/src/matrix_c.cpp
)

target_include_directories(matrix
    PUBLIC
        native/include
)

set_target_properties(matrix PROPERTIES
    OUTPUT_NAME "matrix"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ---- JNI shim: thin bridge for JVM via System.load(…) / loadLibrary(…) ------
find_package(JNI REQUIRED)

add_library(matrix_jni SHARED
    native/jni/matrix_jni.cpp
)

target_include_directories(matrix_jni
    PRIVATE
        ${JNI_INCLUDE_DIRS}
        native/include
)

target_link_libraries(matrix_jni
    PRIVATE
        matrix
        ${JNI_LIBRARIES}
)

set_target_properties(matrix_jni PROPERTIES
    OUTPUT_NAME "matrix_jni"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ---- Make the JNI .so find libmatrix.* next to it at runtime ----------------
# Linux: $ORIGIN is the directory of the loaded library at runtime
# macOS: @loader_path is the directory of the loaded library at runtime
if(APPLE)
    set_target_properties(matrix_jni PROPERTIES
        BUILD_RPATH "@loader_path"
        INSTALL_RPATH "@loader_path"
    )
elseif(UNIX AND NOT WIN32)
    set_target_properties(matrix_jni PROPERTIES
        BUILD_RPATH "\$ORIGIN"
        INSTALL_RPATH "\$ORIGIN"
    )
endif()

# ---- Optional install step (if you ever want: cmake --install native/build) --
install(TARGETS matrix matrix_jni
    RUNTIME DESTINATION lib
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
install(DIRECTORY native/include/ DESTINATION include)
