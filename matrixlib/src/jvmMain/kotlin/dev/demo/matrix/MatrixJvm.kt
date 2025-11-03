package dev.demo.matrix

import dev.demo.matrix.jvm.NativeLoader

/** JVM implementation backed by the JNI bindings generated in native/jni. */
actual class Matrix actual constructor(
    actual val rows: Int,
    actual val cols: Int,
    data: DoubleArray
) : AutoCloseable {

    private var handle: Long

    init {
        // Fail fast on obviously invalid input before we cross the JNI boundary.
        require(rows > 0 && cols > 0) { "rows and cols must be > 0" }
        require(data.size == rows * cols) { "data length mismatch: ${data.size} != ${rows * cols}" }
        handle = NativeLoader.nCreate(rows, cols, data)
    }

    actual val isClosed: Boolean
        get() = handle == 0L

    actual fun toArray(): DoubleArray {
        // Copy the native memory into a managed DoubleArray to avoid exposing
        // raw pointers to callers.
        check(!isClosed) { "Matrix is closed" }
        val out = DoubleArray(rows * cols)
        NativeLoader.nCopyOut(handle, out)
        return out
    }

    actual fun multiply(other: Matrix): Matrix {
        // The native API mutates neither operand and returns a fresh handle for
        // the result.
        check(!isClosed) { "Matrix is closed" }
        val rhs = other as Matrix
        check(!rhs.isClosed) { "Other matrix is closed" }

        val outHandle = NativeLoader.nMultiply(handle, rhs.handle)
        val r = NativeLoader.nRows(outHandle)
        val c = NativeLoader.nCols(outHandle)
        val arr = DoubleArray(r * c)
        NativeLoader.nCopyOut(outHandle, arr)
        NativeLoader.nDestroy(outHandle)

        return Matrix(r, c, arr)
    }

    actual override fun close() {
        if (!isClosed) {
            // The handle is destroyed on the native side and replaced with 0L
            // so repeated calls are harmless.
            NativeLoader.nDestroy(handle)
            handle = 0L
        }
    }
}
