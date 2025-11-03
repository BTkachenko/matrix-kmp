# Matrix KMP Demo

A minimal Kotlin Multiplatform wrapper around a native C++ dense matrix multiplication core.

## Layout

- `native/` – C++ sources, public C header, JNI shim, and CMake project.
- `matrixlib/` – Kotlin Multiplatform library exposing the `Matrix` API to JVM and Kotlin/Native.
- `matrixlib/src/commonMain` – shared Kotlin API surface and error hierarchy.
- `matrixlib/src/jvmMain` – JVM implementation backed by JNI.
- `matrixlib/src/nativeMain` – Kotlin/Native implementation using cinterop.

## Prerequisites

Ensure the following tools are available:

1. **CMake ≥ 3.15** with a C++17-compatible compiler toolchain (clang or gcc on Linux/macOS).
2. **Gradle** installed system-wide (this repository deliberately avoids committing the Gradle wrapper).
3. **Kotlin Multiplatform toolchain** available via Gradle (standard Kotlin plugins will be downloaded when Gradle runs).

> Tip: on a fresh machine run `cmake --version` and `gradle --version` to verify the toolchain before building.

## Build the native libraries

The native C++ core is compiled once and reused by all Gradle tasks.

```bash
cmake -S native -B native/build
cmake --build native/build
```

This produces:

- `native/build/lib/libmatrix.*`
- `native/build/lib/libmatrix_jni.*`

By default the build uses the host triplet (tested on Linux and macOS). Clean the build directory with `rm -rf native/build` if you need to rebuild from scratch.

## Run the test suites

Gradle tasks depend on the native build directory above. From the repository root:

1. **JVM tests** (loads the JNI bridge):
   ```bash
   gradle :matrixlib:jvmTest
   ```
2. **Kotlin/Native tests** for the Linux target:
   ```bash
   gradle :matrixlib:linuxX64Test
   ```

The Native target can be changed (e.g. `macosX64Test`) if the corresponding compiler is installed. Tests expect the native libraries in `native/build/lib`; ensure the native build step succeeded first.

## Usage example

```kotlin
val a = Matrix(2, 3, doubleArrayOf(/* row-major values */))
val b = Matrix(3, 2, doubleArrayOf(/* row-major values */))
val c = a.multiply(b)
println(c.toArray().contentToString())
a.close(); b.close(); c.close()
```

Each `Matrix` owns a native buffer and must be closed when no longer needed. Operations validate
shapes and throw Kotlin exceptions when the underlying native code reports errors. The native
implementation pads inputs to the nearest power of two and applies Strassen's algorithm to keep
asymptotic complexity below the naïve cubic approach while still supporting arbitrary matrix sizes.
