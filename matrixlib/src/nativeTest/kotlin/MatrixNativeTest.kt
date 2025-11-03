import dev.demo.matrix.native.Matrix
import kotlin.test.*


class MatrixNativeTest {
@Test fun multiply2x2_2x2() {
val a = Matrix(2,2, doubleArrayOf(1.0,2.0,3.0,4.0))
val b = Matrix(2,2, doubleArrayOf(5.0,6.0,7.0,8.0))
val c = a.multiply(b)
assertContentEquals(doubleArrayOf(19.0,22.0,43.0,50.0), c.toArray())
a.close(); b.close(); c.close()
}
}