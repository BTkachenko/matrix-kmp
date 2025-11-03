package dev.demo.matrix.jvm


/** Loads JNI library and declares native entry points. */
internal object NativeLoader {
init {
// Expect libmatrix and libmatrix_jni to be on java.library.path / system loader path
System.loadLibrary("matrix")
System.loadLibrary("matrix_jni")
}


@JvmStatic external fun nCreate(rows: Int, cols: Int, data: DoubleArray): Long
@JvmStatic external fun nDestroy(handle: Long)
@JvmStatic external fun nMultiply(a: Long, b: Long): Long
@JvmStatic external fun nRows(h: Long): Int
@JvmStatic external fun nCols(h: Long): Int
@JvmStatic external fun nCopyOut(h: Long, out: DoubleArray)
}