plugins {
    kotlin("multiplatform") version "2.0.20" apply false
}

allprojects {
    repositories { mavenCentral() }
}

/** Jeden task odpalający wszystko naraz */
tasks.register("checkAll") {
    dependsOn(":matrixlib:buildNative", ":matrixlib:jvmTest", ":matrixlib:linuxX64Test")
}
