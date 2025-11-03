package dev.demo.matrix

/**
 * Exception thrown when the native layer reports a dimension mismatch or
 * otherwise incompatible matrix shape.
 */
class NativeShapeException(message: String) : RuntimeException(message)
