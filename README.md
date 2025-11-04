# Matrix KMP Demo

Kotlin Multiplatform wrapper over a native C++ dense matrix core (cache-friendly tiled `O(n^3)` multiply with aggressive compiler optimizations). JVM uses JNI; Kotlin/Native uses cinterop.

---

## Table of contents
- [Project layout](#project-layout)
- [Requirements](#requirements)
  - [Sanity checks](#sanity-checks)
- [Generate Gradle wrapper (8.9)](#generate-gradle-wrapper-89)
  - [Standard method](#standard-method)
  - [Bootstrap method if build configuration fails](#bootstrap-method-if-build-configuration-fails)
- [Build native libraries (C++)](#build-native-libraries-c)
- [Run tests](#run-tests)
- [Benchmarks (JNI/C++ vs pure Kotlin)](#benchmarks-jniC-vs-pure-kotlin)
- [Clean builds](#clean-builds)
- [Troubleshooting](#troubleshooting)
- [Minimal usage (JVM)](#minimal-usage-jvm)
- [License](#license)

---

## Project layout

- `native/` – C++ core, public C header, JNI shim, CMake project.
- `matrixlib/` – Kotlin Multiplatform library (common + JVM + native targets).
- `benchmarks/` – JVM benchmark app (pure Kotlin vs JNI/C++).

---

## Requirements

- **CMake ≥ 3.15** and a **C++17** compiler (GCC/Clang on Linux/macOS; MSVC on Windows).
- **Java 17+**.
- **Gradle 8.9** (use the wrapper; see below).

### Sanity checks

```bash
cmake --version
g++ --version           # or: clang++ --version
java -version
```

---

## Generate Gradle wrapper (8.9)

### Standard method

Recommended: use the wrapper for everything (`./gradlew ...`). Requires a working system Gradle installation.

```bash
gradle wrapper --gradle-version 8.9
chmod +x gradlew
./gradlew --version
```

### Bootstrap method if build configuration fails

If the Gradle wrapper crashes during project configuration, bootstrap it without touching your existing build.

1. **Create minimal temporary build files** from the repo root:
   ```bash
   cat > wrapper.settings.gradle.kts <<'WRAPSET'
   rootProject.name = "wrapper-bootstrap"
   WRAPSET

   cat > wrapper.build.gradle.kts <<'WRAPBUILD'
   tasks.register<Wrapper>("wrapper") {
       gradleVersion = "8.9"
       distributionType = Wrapper.DistributionType.ALL
   }
   WRAPBUILD
   ```
2. **Run the wrapper task using the temporary files**:
   ```bash
   gradle --settings-file wrapper.settings.gradle.kts \
          --build-file wrapper.build.gradle.kts \
          wrapper
   ```
3. **Clean up and finalize**:
   ```bash
   rm wrapper.settings.gradle.kts wrapper.build.gradle.kts
   chmod +x gradlew
   ./gradlew --version
   ```

Afterwards you will have the following files:

- `gradlew`
- `gradlew.bat`
- `gradle/wrapper/gradle-wrapper.jar`
- `gradle/wrapper/gradle-wrapper.properties`

---

## Build native libraries (C++)

From the repo root:

```bash
cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release
```

Artifacts:

- `native/build/lib/libmatrix.*` – C++ core.
- `native/build/lib/libmatrix_jni.*` – JNI bridge.

If you moved or renamed the repo and see a cache mismatch, rebuild from scratch:

```bash
rm -rf native/build
cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release
```

---

## Run tests

All test tasks rely on the native libs from `native/build/lib`.

```bash
./gradlew :matrixlib:jvmTest \
          :matrixlib:linuxX64Test \
          -Ddev.demo.matrix.libdir="$PWD/native/build/lib"
```

Notes:

- On macOS, use `:matrixlib:macosX64Test` or `:matrixlib:macosArm64Test` depending on the toolchain.
- The system property `dev.demo.matrix.libdir` points the JVM to `libmatrix_jni.*`.

---

## Benchmarks (JNI/C++ vs pure Kotlin)

The benchmark compares JNI/C++ against a simple, cache-aware pure Kotlin `O(n^3)` baseline.

Example:

```bash
./gradlew :benchmarks:run \
  -Ddev.demo.matrix.libdir="$PWD/native/build/lib" \
  --args="--sizes=1024,1536 --iterations=3 --warmups=1 --impl=both"
```

Arguments:

- `--sizes=768,1024,1536,...` – comma-separated square sizes (use ≥1024 to amortize JNI/JIT).
- `--iterations=N` – timed iterations per size (default: 3).
- `--warmups=N` – warmups per size (default: 1).
- `--impl=both|native|kotlin` – which implementation(s) to time.

Tips:

- Close background apps, keep the machine on AC power, and pin CPU frequency for more stable numbers.

---

## Clean builds

```bash
./gradlew clean
rm -rf native/build
```

---

## Troubleshooting

### JNI lib not found

- Pass the correct libdir: `-Ddev.demo.matrix.libdir="$PWD/native/build/lib"`.
- Confirm `native/build/lib` contains `libmatrix_jni.*`.

### “Could not find or load main class dev.demo.matrix.bench.BenchMainKt”

Build before running:

```bash
./gradlew :benchmarks:clean :benchmarks:build :benchmarks:run \
          -Ddev.demo.matrix.libdir="$PWD/native/build/lib"
```

### Gradle Kotlin DSL: `java.io.File.pathSeparator` unresolved

```kotlin
import java.io.File as JFile
// then use: JFile.pathSeparator
```

### CMake cache/source mismatch

```bash
rm -rf native/build
cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release
```

### `DefaultArtifactPublicationSet` during configuration

Remove legacy `maven` plugin usage; use `maven-publish` when publishing is needed.

---

## Minimal usage (JVM)

```kotlin
import dev.demo.matrix.Matrix

fun example() {
    val a = Matrix(2, 3, doubleArrayOf(1.0, 2.0, 3.0,  4.0, 5.0, 6.0)) // row-major
    val b = Matrix(3, 2, doubleArrayOf(7.0, 8.0,  9.0, 10.0,  11.0, 12.0))
    val c = a.multiply(b)
    println(c.toArray().contentToString())
    a.close(); b.close(); c.close()
}
```

Exceptions:

- `NativeShapeException` for shape mismatches.
- `IllegalStateException` for invalid/closed handles.

---

## License

Apache License 2.0. See [`LICENSE`](LICENSE).