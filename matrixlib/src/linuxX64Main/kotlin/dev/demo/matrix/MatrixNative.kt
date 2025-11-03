package dev.demo.matrix

import cnames.structs.matrix_handle_t
import kotlinx.cinterop.*
import matrix.*   // mx_create/mx_copy_out/mx_multiply/mx_rows/mx_cols + MX_ERR_*

/**
 * Linux x64 implementation that bridges to the C API exposed by libmatrix via
 * Kotlin/Native interop.
 */
@OptIn(ExperimentalForeignApi::class)
actual class Matrix actual constructor(
    actual val rows: Int,
    actual val cols: Int,
    data: DoubleArray
) : AutoCloseable {

    init {
        // Validate eagerly before calling into the native layer.
        require(rows > 0 && cols > 0) { "rows and cols must be > 0" }
        require(data.size == rows * cols) {
            "data length mismatch: ${data.size} != ${rows * cols}"
        }
    }

    private var handle: CPointer<matrix_handle_t>? = memScoped {
        val out = alloc<CPointerVar<matrix_handle_t>>()
        data.usePinned { pinned ->
            // size_t is represented as ULong on Linux x64, so cast carefully to
            // avoid truncation.
            val err = mx_create(rows.toULong(), cols.toULong(), pinned.addressOf(0), out.ptr)
            if (err != MX_OK) throw mapErr(err)
        }
        out.value
    }

    actual val isClosed: Boolean get() = handle == null

    actual fun toArray(): DoubleArray {
        val h = handle ?: error("Matrix is closed")
        val outArr = DoubleArray(rows * cols)
        outArr.usePinned { pinned ->
            val err = mx_copy_out(h, pinned.addressOf(0), outArr.size.toULong())
            if (err != MX_OK) throw mapErr(err)
        }
        return outArr
    }

    actual fun multiply(other: Matrix): Matrix {
        val h1 = this.handle ?: error("Matrix is closed")
        val h2 = (other as Matrix).handle ?: error("Matrix is closed")
        return memScoped {
            val out = alloc<CPointerVar<matrix_handle_t>>()
            val err = mx_multiply(h1, h2, out.ptr)
            if (err != MX_OK) throw mapErr(err)

            val res = out.value ?: error("native returned null handle")
            val r = mx_rows(res).toInt()
            val c = mx_cols(res).toInt()

            val arr = DoubleArray(r * c)
            arr.usePinned { pinned ->
                val e2 = mx_copy_out(res, pinned.addressOf(0), arr.size.toULong())
                if (e2 != MX_OK) throw mapErr(e2)
            }
            mx_destroy(res)
            Matrix(r, c, arr)
        }
    }

    actual override fun close() {
        handle?.let { mx_destroy(it) }
        handle = null
    }

    // Map native error codes into idiomatic Kotlin exceptions.
    private fun mapErr(code: Int): RuntimeException {
        val msg = mx_strerror(code)?.toKString() ?: "native error $code"
        return when (code) {
            MX_ERR_SHAPE -> IllegalArgumentException(msg)
            MX_ERR_NULL, MX_ERR_STATE -> IllegalStateException(msg)
            MX_ERR_ALLOC -> RuntimeException("alloc: $msg")
            else -> RuntimeException(msg)
        }
    }
}
