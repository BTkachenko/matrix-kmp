package dev.demo.matrix.jvm

object NativeLoader {
    init {
        val dir = System.getProperty("dev.demo.matrix.libdir")?.trim()
        if (!dir.isNullOrEmpty()) {
            System.load("$dir/libmatrix.so")
            System.load("$dir/libmatrix_jni.so")
        } else {
            try { System.loadLibrary("matrix") } catch (_: Throwable) { /* ignore */ }
            System.loadLibrary("matrix_jni")
        }
    }

    @JvmStatic external fun nCreate(rows: Int, cols: Int, data: DoubleArray): Long
    @JvmStatic external fun nDestroy(handle: Long)
    @JvmStatic external fun nMultiply(a: Long, b: Long): Long
    @JvmStatic external fun nRows(handle: Long): Int
    @JvmStatic external fun nCols(handle: Long): Int
    @JvmStatic external fun nCopyOut(handle: Long, out: DoubleArray)
}
