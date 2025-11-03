import java.io.File

plugins {
    kotlin("jvm")        // ← bez wersji
    application
}

repositories {
    mavenCentral()
}

dependencies {
    implementation(project(":matrixlib"))
}

application {
    mainClass.set("dev.demo.matrix.bench.MainKt")
}

tasks.named<JavaExec>("run") {
    dependsOn(":matrixlib:buildNative")

    val libDir = System.getProperty("dev.demo.matrix.libdir")
        ?: "${rootProject.projectDir}/native/build/lib"

    systemProperty("dev.demo.matrix.libdir", libDir)
    environment("LD_LIBRARY_PATH", libDir)
    environment("DYLD_LIBRARY_PATH", libDir)
    environment("PATH", libDir + File.pathSeparator + (System.getenv("PATH") ?: ""))
}
