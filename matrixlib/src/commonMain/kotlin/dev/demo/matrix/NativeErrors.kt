package dev.demo.matrix


open class NativeMatrixException(message: String) : RuntimeException(message)
class NativeShapeException(message: String) : NativeMatrixException(message)