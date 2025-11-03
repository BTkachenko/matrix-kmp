package dev.demo.matrix.jvm


import dev.demo.matrix.Matrix
import dev.demo.matrix.NativeMatrixException
import dev.demo.matrix.NativeShapeException


/** JVM actual using JNI-backed native handle. */
actual class Matrix actual constructor(
actual val rows: Int,
actual val cols: Int,
data: DoubleArray
) {
private var handle: Long


init {
require(rows > 0 && cols > 0) { "rows and cols must be > 0" }
require(data.size == rows * cols) { "data length must be rows*cols" }
// Bridge exceptions thrown from JNI will propagate as Java exceptions.
handle = NativeLoader.nCreate(rows, cols, data)
}


override fun toString(): String = "Matrix(${rows}x${cols}, closed=$isClosed)"


actual fun toArray(): DoubleArray {
check(!isClosed) { "Matrix is closed" }
val out = DoubleArray(rows * cols)
NativeLoader.nCopyOut(handle, out)
return out
}


actual fun multiply(other: Matrix): Matrix {
check(!isClosed) { "Matrix is closed" }
require(other.rows == this.cols) { "shape mismatch: ${this.rows}x${this.cols} * ${other.rows}x${other.cols}" }
val o = other as Matrix
val newHandle = NativeLoader.nMultiply(this.handle, o.handle)
val r = NativeLoader.nRows(newHandle)
val c = NativeLoader.nCols(newHandle)
// Copy out lazily on demand; construct via private ctor
return fromHandle(r, c, newHandle)
}


actual fun close() {
if (!isClosed) {
NativeLoader.nDestroy(handle)
handle = 0L
}
}


actual val isClosed: Boolean get() = handle == 0L


// Private ctor helper for results
private constructor(rows: Int, cols: Int, handle: Long) : this(rows, cols, DoubleArray(rows * cols)) {
// Bypass creation – we immediately replace the handle
this.handle = handle
}


private companion object {
fun fromHandle(rows: Int, cols: Int, handle: Long): Matrix = Matrix(rows, cols, handle)
}
}