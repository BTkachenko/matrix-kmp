plugins {
kotlin("multiplatform")
}


kotlin {
jvm()


// Native targets; enable what you need locally
macosX64()
macosArm64()
linuxX64()
mingwX64()


sourceSets {
val commonMain by getting
val commonTest by getting { dependencies { implementation(kotlin("test")) } }


val jvmMain by getting
val jvmTest by getting { dependencies { implementation(kotlin("test")) } }


val nativeMain by creating { dependsOn(commonMain) }
val nativeTest by creating {
dependsOn(commonTest)
}


targets.withType<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget> {
compilations["main"].defaultSourceSet.dependsOn(nativeMain)
compilations["test"].defaultSourceSet.dependsOn(nativeTest)
}
}


// cinterop for native targets
targets.withType<org.jetbrains.kotlin.gradle.plugin.mpp.KotlinNativeTarget>().configureEach {
compilations.getByName("main") {
cinterops.create("matrix") {
defFile(project.file("src/nativeInterop/cinterop/matrix.def"))
}
// Linker opts: adjust path to built native library dir
val libDir = project.rootProject.file("native/build").resolve("lib").absolutePath
binaries.all { linkerOpts("-L$libDir", "-lmatrix") }
}
}
}