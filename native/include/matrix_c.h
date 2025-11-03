#pragma once
#include <stddef.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


// Opaque handle type; implementation detail hidden from consumers.
typedef struct matrix_handle_t matrix_handle_t;


// Error codes (0 = OK)
#define MX_OK 0
#define MX_ERR_NULL 1
#define MX_ERR_SHAPE 2
#define MX_ERR_ALLOC 3
#define MX_ERR_STATE 4
#define MX_ERR_UNKNOWN 255


// Utility: map error code to static string (no allocations).
const char* mx_strerror(int code);


// Create matrix from raw row-major buffer. Copies input data.
int mx_create(size_t rows, size_t cols, const double* data, matrix_handle_t** out);


// Destroy matrix and free memory. Safe to call with NULL.
void mx_destroy(matrix_handle_t* h);


// Multiply A * B, allocate new matrix into *out.
int mx_multiply(const matrix_handle_t* a, const matrix_handle_t* b, matrix_handle_t** out);


// Accessors
size_t mx_rows(const matrix_handle_t* h);
size_t mx_cols(const matrix_handle_t* h);


// Copy out data into caller-provided buffer of length rows*cols.
int mx_copy_out(const matrix_handle_t* h, double* out, size_t out_len);


#ifdef __cplusplus
}
#endif