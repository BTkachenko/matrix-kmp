package dev.demo.matrix

/**
 * Wspólny kontrakt (expect/actual).
 */
expect class Matrix(rows: Int, cols: Int, data: DoubleArray) : AutoCloseable {
    val rows: Int
    val cols: Int
    val isClosed: Boolean
    fun toArray(): DoubleArray
    fun multiply(other: Matrix): Matrix
    override fun close()
}
