package dev.demo.matrix.jvm

/**
 * Loads the native matrix libraries and exposes the JNI entry points consumed
 * by [MatrixJvm].
 */
object NativeLoader {
    init {
        val dir = System.getProperty("dev.demo.matrix.libdir")?.trim()
        if (!dir.isNullOrEmpty()) {
            System.load("$dir/libmatrix.so")
            System.load("$dir/libmatrix_jni.so")
        } else {
            // Try the default lookup path first so development environments can
            // provide the library through java.library.path.
            try { System.loadLibrary("matrix") } catch (_: Throwable) { /* ignore */ }
            System.loadLibrary("matrix_jni")
        }
    }

    // Native constructors/destructors and helpers used by MatrixJvm.
    @JvmStatic external fun nCreate(rows: Int, cols: Int, data: DoubleArray): Long
    @JvmStatic external fun nDestroy(handle: Long)
    @JvmStatic external fun nMultiply(a: Long, b: Long): Long
    @JvmStatic external fun nRows(handle: Long): Int
    @JvmStatic external fun nCols(handle: Long): Int
    @JvmStatic external fun nCopyOut(handle: Long, out: DoubleArray)
}
