package dev.demo.matrix


/** Common, clean API surface. Each platform holds a native handle and owns its buffer. */
expect class Matrix(rows: Int, cols: Int, data: DoubleArray) {
val rows: Int
val cols: Int
fun toArray(): DoubleArray
fun multiply(other: Matrix): Matrix
fun close()
val isClosed: Boolean
}