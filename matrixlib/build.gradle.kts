import java.io.File
import org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget
import org.jetbrains.kotlin.gradle.targets.native.tasks.KotlinNativeTest

plugins {
    kotlin("multiplatform")
}

val nativeBuildDir = rootProject.layout.projectDirectory.dir("native/build")
val nativeLibDir = nativeBuildDir.dir("lib").asFile

val configureNative by tasks.registering(Exec::class) {
    workingDir = rootProject.projectDir
    commandLine("cmake", "-S", ".", "-B", "native/build", "-DCMAKE_BUILD_TYPE=Release")
}

val buildNative by tasks.registering(Exec::class) {
    dependsOn(configureNative)
    workingDir = rootProject.projectDir
    commandLine("cmake", "--build", "native/build", "--config", "Release")
}

kotlin {
    jvm()
    linuxX64()
    macosX64()
    macosArm64()
    mingwX64()

    sourceSets {
        val commonMain by getting
        val commonTest by getting { dependencies { implementation(kotlin("test")) } }
        val jvmMain by getting
        val jvmTest by getting { dependencies { implementation(kotlin("test")) } }
        val nativeMain by creating { dependsOn(commonMain) }
        val nativeTest by creating { dependsOn(commonTest) }

        targets.withType<KotlinNativeTarget> {
            compilations["main"].defaultSourceSet.dependsOn(nativeMain)
            compilations["test"].defaultSourceSet.dependsOn(nativeTest)
        }
    }
}

kotlin.targets.withType<KotlinNativeTarget>().configureEach {
    compilations.getByName("main") {
        cinterops.create("matrix") {
            defFile(project.file("src/nativeInterop/cinterop/matrix.def"))
            includeDirs(project.rootProject.file("native/include"))
        }
    }
    val libDir = project.rootProject.file("native/build").resolve("lib").absolutePath
    binaries.all {
        linkerOpts("-L$libDir", "-lmatrix")
        linkTaskProvider.configure { dependsOn(buildNative) }
    }
}

tasks.withType<Test>().configureEach {
    dependsOn(buildNative)
    systemProperty("java.library.path", nativeLibDir.absolutePath)
    environment("PATH", nativeLibDir.absolutePath + File.pathSeparator + (System.getenv("PATH") ?: ""))
    environment("LD_LIBRARY_PATH", nativeLibDir.absolutePath)
    environment("DYLD_LIBRARY_PATH", nativeLibDir.absolutePath)
    systemProperty("dev.demo.matrix.libdir", nativeLibDir.absolutePath)
}

tasks.withType<KotlinNativeTest>().configureEach {
    dependsOn(buildNative)
    environment("LD_LIBRARY_PATH", nativeLibDir.absolutePath)
    environment("DYLD_LIBRARY_PATH", nativeLibDir.absolutePath)
    environment("PATH", nativeLibDir.absolutePath + File.pathSeparator + (System.getenv("PATH") ?: ""))
}
