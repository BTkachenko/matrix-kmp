package dev.demo.matrix.native


import dev.demo.matrix.Matrix
import dev.demo.matrix.NativeMatrixException
import dev.demo.matrix.NativeShapeException
import kotlin.native.internal.NativePtr
import kotlinx.cinterop.*
import matrix.*


actual class Matrix actual constructor(
actual val rows: Int,
actual val cols: Int,
data: DoubleArray
) {
private var handle: CPointer<matrix_handle_t>? = null


init {
require(rows > 0 && cols > 0) { "rows and cols must be > 0" }
require(data.size == rows * cols) { "data length must be rows*cols" }
memScoped {
val out = alloc<CPointerVar<matrix_handle_t>>()
data.usePinned { pinned ->
val err = mx_create(rows.convert(), cols.convert(), pinned.addressOf(0), out.ptr)
if (err != MX_OK) throw mapErr(err)
}
handle = out.value
}
}


actual fun toArray(): DoubleArray {
check(!isClosed) { "Matrix is closed" }
val out = DoubleArray(rows * cols)
out.usePinned { pinned ->
val err = mx_copy_out(handle, pinned.addressOf(0), out.size.convert())
if (err != MX_OK) throw mapErr(err)
}
return out
}


actual fun multiply(other: Matrix): Matrix {
check(!isClosed) { "Matrix is closed" }
require(other.rows == this.cols) { "shape mismatch: ${this.rows}x${this.cols} * ${other.rows}x${other.cols}" }
val o = other as Matrix
memScoped {
val outH = alloc<CPointerVar<matrix_handle_t>>()
val err = mx_multiply(handle, o.handle, outH.ptr)
if (err != MX_OK) throw mapErr(err)
val r = mx_rows(outH.value)
val c = mx_cols(outH.value)
return fromHandle(r.toInt(), c.toInt(), outH.value)
}
}


actual fun close() {
handle?.let { mx_destroy(it) }
handle = null
}


actual val isClosed: Boolean get() = handle == null


private constructor(rows: Int, cols: Int, h: CPointer<matrix_handle_t>?) : this(rows, cols, DoubleArray(rows * cols)) {
handle = h
}
}