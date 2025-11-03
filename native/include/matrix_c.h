#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Lightweight C interface that wraps the C++ DenseMatrix implementation. All
// operations allocate and free opaque handles so that consumers do not depend
// on the underlying data structures.

// Opaque handle type; implementation detail hidden from consumers.
typedef struct matrix_handle_t matrix_handle_t;

// Error codes (0 = OK)
#define MX_OK 0
#define MX_ERR_NULL 1
#define MX_ERR_SHAPE 2
#define MX_ERR_ALLOC 3
#define MX_ERR_STATE 4
#define MX_ERR_UNKNOWN 255

/**
 * Return a pointer to a static, human-readable string describing the error
 * code produced by the API. The caller must not free or modify the returned
 * pointer.
 */
const char* mx_strerror(int code);

/**
 * Allocate a new matrix and copy the provided row-major buffer into it.
 *
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 * @param data Pointer to a buffer containing rows*cols doubles.
 * @param out  Receives the newly created handle when the call succeeds.
 * @return MX_OK on success or one of MX_ERR_* on failure.
 */
int mx_create(size_t rows, size_t cols, const double* data, matrix_handle_t** out);

/**
 * Destroy a matrix handle previously obtained from mx_create or mx_multiply.
 * Passing NULL is a no-op.
 */
void mx_destroy(matrix_handle_t* h);

/**
 * Multiply two matrices and return the product as a new handle.
 * The caller assumes ownership of the returned handle and must eventually
 * release it via mx_destroy.
 */
int mx_multiply(const matrix_handle_t* a, const matrix_handle_t* b, matrix_handle_t** out);

/**
 * Query the number of rows or columns tracked by a matrix handle. Returns 0
 * when the handle is null to help defensive callers detect invalid inputs.
 */
size_t mx_rows(const matrix_handle_t* h);
size_t mx_cols(const matrix_handle_t* h);

/**
 * Copy the matrix contents into a caller-provided buffer.
 *
 * @param h       Matrix handle to copy from.
 * @param out     Destination buffer for rows*cols doubles.
 * @param out_len Number of elements available in the destination buffer.
 * @return MX_OK on success, MX_ERR_NULL for null output, or MX_ERR_SHAPE when
 *         out_len does not match the matrix size.
 */
int mx_copy_out(const matrix_handle_t* h, double* out, size_t out_len);

#ifdef __cplusplus
}
#endif

