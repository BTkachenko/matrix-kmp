package dev.demo.matrix

import dev.demo.matrix.jvm.NativeLoader

actual class Matrix actual constructor(
    actual val rows: Int,
    actual val cols: Int,
    data: DoubleArray
) : AutoCloseable {

    private var handle: Long

    init {
        require(rows > 0 && cols > 0) { "rows and cols must be > 0" }
        require(data.size == rows * cols) { "data length mismatch: ${data.size} != ${rows * cols}" }
        handle = NativeLoader.nCreate(rows, cols, data)
    }

    actual val isClosed: Boolean
        get() = handle == 0L

    actual fun toArray(): DoubleArray {
        check(!isClosed) { "Matrix is closed" }
        val out = DoubleArray(rows * cols)
        NativeLoader.nCopyOut(handle, out)
        return out
    }

    actual fun multiply(other: Matrix): Matrix {
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
            NativeLoader.nDestroy(handle)
            handle = 0L
        }
    }
}
