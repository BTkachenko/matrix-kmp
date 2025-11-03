# Matrix KMP Demo

Kotlin Multiplatform wrapper over a native C++ dense matrix core (cache-friendly tiled `O(n^3)` multiply with aggressive compiler optimizations). JVM uses JNI; Kotlin/Native uses cinterop.

---

## Table of contents
1. [Project layout](#project-layout)
2. [Requirements](#requirements)
3. [Generate Gradle wrapper (8.9)](#generate-gradle-wrapper-89)
   - [Bootstrap method if your build scripts fail to configure](#bootstrap-method-if-your-build-scripts-fail-to-configure)
4. [Build native libraries (C++)](#build-native-libraries-c)
5. [Run tests](#run-tests)
6. [Benchmarks (JNI/C++ vs pure Kotlin)](#benchmarks-jniC-vs-pure-kotlin)
7. [Clean builds](#clean-builds)
8. [Troubleshooting](#troubleshooting)
9. [Minimal usage (JVM)](#minimal-usage-jvm)
10. [License](#license)

---

## Project layout

native/ # C++ core, public C header, JNI shim, CMake project
matrixlib/ # Kotlin Multiplatform library (common + jvm + native targets)
benchmarks/ # JVM benchmark app (pure Kotlin vs JNI/C++)


---

## Requirements

- **CMake ≥ 3.15** and a **C++17** compiler (GCC/Clang on Linux/macOS; MSVC on Windows).
- **Java 17+**.
- **Gradle 8.9** (use the wrapper; see below).

Sanity checks:
```bash
cmake --version
g++ --version           # or: clang++ --version
java -version

Generate Gradle wrapper (8.9)

    Recommended: use the wrapper for everything (./gradlew ...).

Standard way (requires a working system Gradle):

gradle wrapper --gradle-version 8.9
chmod +x gradlew
./gradlew --version

Bootstrap method if your build scripts fail to configure

If gradle wrapper crashes during project configuration, bootstrap the wrapper without touching your existing build:

# From repo root

# 1) Create minimal temporary build files:
cat > wrapper.settings.gradle.kts <<'EOF'
rootProject.name = "wrapper-bootstrap"
EOF

cat > wrapper.build.gradle.kts <<'EOF'
tasks.register<Wrapper>("wrapper") {
    gradleVersion = "8.9"
    distributionType = Wrapper.DistributionType.ALL
}
EOF

# 2) Run wrapper task using the temporary files:
gradle --settings-file wrapper.settings.gradle.kts \
       --build-file wrapper.build.gradle.kts \
       wrapper

# 3) Clean up and finalize:
rm wrapper.settings.gradle.kts wrapper.build.gradle.kts
chmod +x gradlew
./gradlew --version

You now have:

gradlew
gradlew.bat
gradle/wrapper/gradle-wrapper.jar
gradle/wrapper/gradle-wrapper.properties

Build native libraries (C++)

From repo root:

cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release

Artifacts:

native/build/lib/libmatrix.*        # C++ core
native/build/lib/libmatrix_jni.*    # JNI bridge

If you moved/renamed the repo and see a cache mismatch, rebuild from scratch:

rm -rf native/build
cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
cmake --build native/build --config Release

Run tests

All test tasks rely on the native libs from native/build/lib.

./gradlew :matrixlib:jvmTest \
          :matrixlib:linuxX64Test \
          -Ddev.demo.matrix.libdir="$PWD/native/build/lib"

Notes:

    On macOS, use :matrixlib:macosX64Test or :matrixlib:macosArm64Test depending on the toolchain.

    The system property dev.demo.matrix.libdir points the JVM to libmatrix_jni.*.

Benchmarks (JNI/C++ vs pure Kotlin)

The benchmark compares JNI/C++ against a simple, cache-aware pure Kotlin O(n^3) baseline.

Example:

./gradlew :benchmarks:run \
  -Ddev.demo.matrix.libdir="$PWD/native/build/lib" \
  --args="--sizes=1024,1536 --iterations=3 --warmups=1 --impl=both"

Arguments:

    --sizes=768,1024,1536,... comma-separated square sizes (use ≥1024 to amortize JNI/JIT).

    --iterations=N timed iterations per size (default: 3).

    --warmups=N warmups per size (default: 1).

    --impl=both|native|kotlin which implementation(s) to time.

Tips:

    Close background apps, keep the machine on AC power, and pin CPU frequency for more stable numbers.

Clean builds

./gradlew clean
rm -rf native/build

Troubleshooting

JNI lib not found

    Pass the correct libdir: -Ddev.demo.matrix.libdir="$PWD/native/build/lib".

    Confirm native/build/lib contains libmatrix_jni.*.

“Could not find or load main class dev.demo.matrix.bench.BenchMainKt”

    Build before run:

    ./gradlew :benchmarks:clean :benchmarks:build :benchmarks:run \
      -Ddev.demo.matrix.libdir="$PWD/native/build/lib"

Gradle Kotlin DSL: java.io.File.pathSeparator unresolved

    In the Gradle script:

    import java.io.File as JFile
    // then use: JFile.pathSeparator

CMake cache/source mismatch

    Rebuild native:

    rm -rf native/build
    cmake -S . -B native/build -DCMAKE_BUILD_TYPE=Release
    cmake --build native/build --config Release

DefaultArtifactPublicationSet during configuration

    Remove legacy maven plugin usage; use maven-publish when publishing is needed.

Minimal usage (JVM)

import dev.demo.matrix.Matrix

fun example() {
    val a = Matrix(2, 3, doubleArrayOf(1.0, 2.0, 3.0,  4.0, 5.0, 6.0)) // row-major
    val b = Matrix(3, 2, doubleArrayOf(7.0, 8.0,  9.0, 10.0,  11.0, 12.0))
    val c = a.multiply(b)
    println(c.toArray().contentToString())
    a.close(); b.close(); c.close()
}

Exceptions:

    NativeShapeException for shape mismatches

    IllegalStateException for invalid/closed handles

    RuntimeException for allocation/unknown errors

License

MIT (or your project’s chosen license).

