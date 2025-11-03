import dev.demo.matrix.Matrix
fun main() {
    val a = Matrix(2,2,doubleArrayOf(1.0,2.0,3.0,4.0))
    val b = Matrix(2,2,doubleArrayOf(5.0,6.0,7.0,8.0))
    val c = a.multiply(b)
    println(c.toArray().contentToString())  // expect: [19.0, 22.0, 43.0, 50.0]
    a.close(); b.close(); c.close()
}