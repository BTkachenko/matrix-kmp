package dev.demo.matrix.bench

import dev.demo.matrix.Matrix
import kotlin.math.abs
import kotlin.system.measureNanoTime
import java.util.Random

private data class Params(
    val sizes: List<Int>,
    val iterations: Int,
    val warmups: Int,
    val impl: String // "native" | "kotlin" | "both"
)

private fun parseArgs(args: Array<String>): Params {
    fun get(name: String, def: String) =
        args.firstOrNull { it.startsWith("--$name=") }?.substringAfter("=") ?: def

    val sizes = get("sizes", "768")
        .split(',').map { it.trim().toInt() }.filter { it > 0 }
    val iterations = get("iterations", "3").toInt()
    val warmups = get("warmups", "1").toInt()
    val impl = get("impl", "both").lowercase()
    return Params(sizes, iterations, warmups, impl)
}

// Prosta, cache-friendly implementacja O(n^3) w czystym Kotlinie (row-major).
private fun mulKotlin(a: DoubleArray, b: DoubleArray, n: Int): DoubleArray {
    val c = DoubleArray(n * n)
    val nLong = n
    var i = 0
    while (i < nLong) {
        val crow = i * nLong
        var k = 0
        while (k < nLong) {
            val aik = a[i * nLong + k]
            val brow = k * nLong
            var j = 0
            while (j < nLong) {
                c[crow + j] += aik * b[brow + j]
                j++
            }
            k++
        }
        i++
    }
    return c
}

private fun checksum10(a: DoubleArray): Double {
    var s = 0.0
    val lim = minOf(10, a.size)
    for (i in 0 until lim) s += a[i]
    return s
}

private fun approxEqual(a: DoubleArray, b: DoubleArray, eps: Double = 1e-9): Boolean {
    if (a.size != b.size) return false
    for (i in a.indices) {
        if (abs(a[i] - b[i]) > eps) return false
    }
    return true
}

fun main(args: Array<String>) {
    val p = parseArgs(args)
    println("=== Matrix Benchmarks (JNI/C++ vs Pure Kotlin) ===")
    println("sizes=${p.sizes}, iterations=${p.iterations}, warmups=${p.warmups}, impl=${p.impl}")
    val rnd = Random(1234567L)

    for (n in p.sizes) {
        println("\n--- n=$n ---")

        // Generujemy te same dane do obu implementacji
        fun randomMatrix(n: Int): DoubleArray {
            val out = DoubleArray(n * n)
            for (i in out.indices) out[i] = rnd.nextDouble() * 2.0 - 1.0
            return out
        }
        val aArr = randomMatrix(n)
        val bArr = randomMatrix(n)

        // Sanity check na małej macierzy (np. 64) – jednorazowo
        if (n == p.sizes.first() && (p.impl == "both")) {
            val testN = minOf(64, n)
            val aT = aArr.copyOf(testN * testN)
            val bT = bArr.copyOf(testN * testN)
            val nativeC = Matrix(testN, testN, aT).use { A ->
                Matrix(testN, testN, bT).use { B ->
                    A.multiply(B).use { C -> C.toArray() }
                }
            }
            val kotlinC = mulKotlin(aT, bT, testN)
            val ok = approxEqual(nativeC, kotlinC, 1e-7)
            println("Sanity (n=$testN) equal within 1e-7: $ok  (chk native=${checksum10(nativeC)}, kotlin=${checksum10(kotlinC)})")
        }

        if (p.impl == "native" || p.impl == "both") {
            // Prealokacja obiektów, żeby nie liczyć ich kosztu w pętli
            val A = Matrix(n, n, aArr)
            val B = Matrix(n, n, bArr)
            try {
                // Warm-up
                repeat(p.warmups) {
                    Matrix(n, n, aArr).use { _ -> } // drobny ruch JVM/JIT
                    A.multiply(B).use { C -> checksum10(C.toArray()) }
                }
                // Pomiary
                val timesMs = DoubleArray(p.iterations)
                var blackhole = 0.0
                repeat(p.iterations) { idx ->
                    val t = measureNanoTime {
                        A.multiply(B).use { C ->
                            // konsumujemy wynik
                            blackhole += checksum10(C.toArray())
                        }
                    }
                    timesMs[idx] = t / 1e6
                }
                println("Native (JNI/C++): avg=${"%.2f".format(timesMs.average())} ms, " +
                        "best=${"%.2f".format(timesMs.minOrNull()!!)} ms, " +
                        "worst=${"%.2f".format(timesMs.maxOrNull()!!)} ms, blackhole=$blackhole")
            } finally {
                A.close()
                B.close()
            }
        }

        if (p.impl == "kotlin" || p.impl == "both") {
            // Warm-up
            repeat(p.warmups) { mulKotlin(aArr, bArr, n) }
            val timesMs = DoubleArray(p.iterations)
            var blackhole = 0.0
            repeat(p.iterations) { idx ->
                val t = measureNanoTime {
                    val C = mulKotlin(aArr, bArr, n)
                    blackhole += checksum10(C)
                }
                timesMs[idx] = t / 1e6
            }
            println("Pure Kotlin O(n^3): avg=${"%.2f".format(timesMs.average())} ms, " +
                    "best=${"%.2f".format(timesMs.minOrNull()!!)} ms, " +
                    "worst=${"%.2f".format(timesMs.maxOrNull()!!)} ms, blackhole=$blackhole")
        }
    }
    println("\nDone.")
}

// Ułatwienie .use dla Matrix (AutoCloseable)
private inline fun <T : AutoCloseable, R> T.use(block: (T) -> R): R {
    var thrown: Throwable? = null
    try {
        return block(this)
    } catch (t: Throwable) {
        thrown = t
        throw t
    } finally {
        try { this.close() } catch (closeEx: Throwable) {
            if (thrown == null) throw closeEx
        }
    }
}
