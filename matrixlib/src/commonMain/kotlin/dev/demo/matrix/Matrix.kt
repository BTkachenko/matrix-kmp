package dev.demo.matrix

/**
 * Platform-agnostic contract for a dense double matrix backed by the native
 * library. Implementations manage a native handle and expose convenience
 * methods for copying data and performing multiplication.
 */
expect class Matrix(rows: Int, cols: Int, data: DoubleArray) : AutoCloseable {
    /** Number of rows represented by the matrix. */
    val rows: Int
    /** Number of columns represented by the matrix. */
    val cols: Int
    /** True when the native resources have been released. */
    val isClosed: Boolean
    /** Copy the matrix contents into a JVM/Kotlin-owned array. */
    fun toArray(): DoubleArray
    /**
     * Multiply this matrix by another one, returning a new matrix that owns its
     * own native resources.
     */
    fun multiply(other: Matrix): Matrix
    /** Release the underlying native resources. */
    override fun close()
}
