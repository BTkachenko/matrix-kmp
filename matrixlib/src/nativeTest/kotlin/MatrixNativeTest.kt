package dev.demo.matrix

import kotlin.test.*

class MatrixNativeTest {

    @Test
    fun multiply2x3_3x2() {
        // Arrange two matrices whose product matches the JVM variant exactly.
        val a = Matrix(2, 3, doubleArrayOf(
            1.0, 2.0, 3.0,
            4.0, 5.0, 6.0
        ))
        val b = Matrix(3, 2, doubleArrayOf(
            7.0, 8.0,
            9.0, 10.0,
            11.0, 12.0
        ))

        val c = a.multiply(b)
        val arr = c.toArray()
        assertContentEquals(doubleArrayOf(58.0, 64.0, 139.0, 154.0), arr)
        a.close(); b.close(); c.close()
    }

    @Test
    fun shapeMismatchThrows() {
        // The native implementation should mirror the JVM error semantics.
        val a = Matrix(2, 2, doubleArrayOf(1.0, 2.0, 3.0, 4.0))
        val b = Matrix(3, 1, doubleArrayOf(5.0, 6.0, 7.0))
        assertFailsWith<IllegalArgumentException> { a.multiply(b) }
        a.close(); b.close()
    }

    @Test
    fun closedSafety() {
        // Verify that closing the native handle prevents further access.
        val a = Matrix(1, 1, doubleArrayOf(42.0))
        a.close()
        assertTrue(a.isClosed)
        assertFailsWith<IllegalStateException> { a.toArray() }
    }
}
