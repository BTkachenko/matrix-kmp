import org.jetbrains.kotlin.gradle.targets.native.tasks.KotlinNativeTest

plugins {
    kotlin("multiplatform")
}

// === Native build via CMake (JNI + libmatrix) ===
val nativeBuildDir = rootProject.layout.projectDirectory.dir("native/build")
val nativeLibDir = nativeBuildDir.dir("lib").asFile

val configureNative by tasks.registering(Exec::class) {
    workingDir = rootProject.projectDir
    commandLine("cmake", "-S", ".", "-B", "native/build")
}

val buildNative by tasks.registering(Exec::class) {
    dependsOn(configureNative)
    workingDir = rootProject.projectDir
    commandLine("cmake", "--build", "native/build")
}

kotlin {
    // JVM (JNI)
    jvm()

    // Linux K/N (cinterop do C API)
    linuxX64()

    sourceSets {
        val commonMain by getting
        val commonTest by getting {
            dependencies { implementation(kotlin("test")) }
        }

        val jvmMain by getting
        val jvmTest by getting {
            dependencies { implementation(kotlin("test")) }
        }

        // Trzymamy natywne źródła w linuxX64Main / linuxX64Test
        val linuxX64Main by getting
        val linuxX64Test by getting {
            dependencies { implementation(kotlin("test")) }
        }
    }

    // >>> KLUCZ: cinterop przypięty do linuxX64/main <<<
    linuxX64 {
        compilations.getByName("main") {
            cinterops {
                create("matrix") {
                    // Def plik: matrix.def
                    defFile(project.file("src/nativeInterop/cinterop/matrix.def"))
                    // Nagłówki C API
                    includeDirs(project.rootProject.file("native/include"))
                }
            }
        }

        // Link do libmatrix.so (z CMake)
        val libDir = project.rootProject.file("native/build/lib").absolutePath
        binaries.all {
            linkerOpts("-L$libDir", "-lmatrix")
            // Zawsze zbuduj CMake przed linkiem K/N
            linkTaskProvider.configure { dependsOn(buildNative) }
        }
    }
}

// Ustawienia środowiska dla testów JVM (JNI) i K/N
tasks.withType<Test>().configureEach {
    dependsOn(buildNative)

    // JVM: ścieżka do natywnych .so
    systemProperty(
        "java.library.path",
        nativeLibDir.absolutePath
    )

    // Domyślny libdir (można nadpisać -Ddev.demo.matrix.libdir=...)
    systemProperty(
        "dev.demo.matrix.libdir",
        System.getProperty("dev.demo.matrix.libdir") ?: nativeLibDir.absolutePath
    )

    // Dla loaderów natywnych
    environment("PATH", nativeLibDir.absolutePath + File.pathSeparator + (environment["PATH"] ?: ""))
    environment("LD_LIBRARY_PATH", nativeLibDir.absolutePath)
    environment("DYLD_LIBRARY_PATH", nativeLibDir.absolutePath)
}

tasks.withType<KotlinNativeTest>().configureEach {
    dependsOn(buildNative)
    environment("PATH", nativeLibDir.absolutePath + File.pathSeparator + (environment["PATH"] ?: ""))
    environment("LD_LIBRARY_PATH", nativeLibDir.absolutePath)
    environment("DYLD_LIBRARY_PATH", nativeLibDir.absolutePath)
}
