#include <jni.h>
#include "matrix_c.h"

// Helper: throw Kotlin/Java exceptions by FQN.
static void throwJava(JNIEnv* env, const char* className, const char* msg) {
    jclass cls = env->FindClass(className);
    if (!cls) cls = env->FindClass("java/lang/RuntimeException");
    env->ThrowNew(cls, msg);
}

static void throwFromErr(JNIEnv* env, int err) {
    switch (err) {
        case MX_ERR_SHAPE:
            // JVM test oczekuje IllegalArgumentException dla błędu kształtu.
            throwJava(env, "java/lang/IllegalArgumentException", mx_strerror(err));
            break;
        case MX_ERR_NULL:
        case MX_ERR_STATE:
            throwJava(env, "java/lang/IllegalStateException", mx_strerror(err));
            break;
        case MX_ERR_ALLOC:
            throwJava(env, "java/lang/OutOfMemoryError", mx_strerror(err));
            break;
        default:
            throwJava(env, "java/lang/RuntimeException", mx_strerror(err));
            break;
    }
}

extern "C" JNIEXPORT jlong JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nCreate(JNIEnv* env, jclass,
                                              jint rows, jint cols, jdoubleArray data) {
    if (!data) { throwJava(env, "java/lang/NullPointerException", "data is null"); return 0; }
    jsize len = env->GetArrayLength(data);
    const jsize need = static_cast<jsize>(static_cast<long long>(rows) * static_cast<long long>(cols));
    if (len != need) {
        throwJava(env, "java/lang/IllegalArgumentException", "data length mismatch");
        return 0;
    }
    jboolean isCopy = JNI_FALSE;
    jdouble* ptr = env->GetDoubleArrayElements(data, &isCopy);
    matrix_handle_t* h = nullptr;
    int err = mx_create((size_t)rows, (size_t)cols, (const double*)ptr, &h);
    env->ReleaseDoubleArrayElements(data, ptr, JNI_ABORT);
    if (err != MX_OK) { throwFromErr(env, err); return 0; }
    return reinterpret_cast<jlong>(h);
}

extern "C" JNIEXPORT void JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nDestroy(JNIEnv*, jclass, jlong handle) {
    mx_destroy(reinterpret_cast<matrix_handle_t*>(handle));
}

extern "C" JNIEXPORT jlong JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nMultiply(JNIEnv* env, jclass, jlong a, jlong b) {
    matrix_handle_t* out = nullptr;
    int err = mx_multiply(reinterpret_cast<matrix_handle_t*>(a),
                          reinterpret_cast<matrix_handle_t*>(b), &out);
    if (err != MX_OK) { throwFromErr(env, err); return 0; }
    return reinterpret_cast<jlong>(out);
}

extern "C" JNIEXPORT jint JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nRows(JNIEnv*, jclass, jlong h) {
    return (jint) mx_rows(reinterpret_cast<matrix_handle_t*>(h));
}

extern "C" JNIEXPORT jint JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nCols(JNIEnv*, jclass, jlong h) {
    return (jint) mx_cols(reinterpret_cast<matrix_handle_t*>(h));
}

extern "C" JNIEXPORT void JNICALL
Java_dev_demo_matrix_jvm_NativeLoader_nCopyOut(JNIEnv* env, jclass, jlong h, jdoubleArray out) {
    if (!out) { throwJava(env, "java/lang/NullPointerException", "out is null"); return; }
    const jint len = env->GetArrayLength(out);
    jboolean isCopy = JNI_FALSE;
    jdouble* ptr = env->GetDoubleArrayElements(out, &isCopy);
    int err = mx_copy_out(reinterpret_cast<matrix_handle_t*>(h), (double*)ptr, (size_t)len);
    env->ReleaseDoubleArrayElements(out, ptr, 0);
    if (err != MX_OK) throwFromErr(env, err);
}
