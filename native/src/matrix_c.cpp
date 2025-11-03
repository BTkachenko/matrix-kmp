#include "matrix_c.h"
#include "matrix_core.hpp"

#include <algorithm>
#include <limits>
#include <memory>
#include <new>
#include <utility>

using matrix::DenseMatrix;
using matrix::shape_error;

// Concrete representation of the opaque matrix handle used by the C API.
struct matrix_handle_t {
    std::unique_ptr<DenseMatrix> matrix;
};

// Translate a numeric error code to its textual form.
const char* mx_strerror(int code) {
    switch (code) {
    case MX_OK: return "OK";
    case MX_ERR_NULL: return "null argument";
    case MX_ERR_SHAPE: return "shape mismatch";
    case MX_ERR_ALLOC: return "allocation failure";
    case MX_ERR_STATE: return "invalid matrix handle";
    default: return "unknown error";
    }
}

// Ensure the provided dimensions are valid and do not overflow size_t when
// multiplied.
static int validate_dimensions(std::size_t rows, std::size_t cols) {
    if (rows == 0 || cols == 0) {
        return MX_ERR_SHAPE;
    }
    const unsigned long long prod = static_cast<unsigned long long>(rows) *
                                    static_cast<unsigned long long>(cols);
    if (prod > static_cast<unsigned long long>(std::numeric_limits<std::size_t>::max())) {
        return MX_ERR_SHAPE;
    }
    return MX_OK;
}

// Create a handle that owns a DenseMatrix copy of the provided buffer.
int mx_create(size_t rows, size_t cols, const double* data, matrix_handle_t** out) {
    if (!out) {
        return MX_ERR_NULL;
    }
    *out = nullptr;
    if (!data) {
        return MX_ERR_NULL;
    }
    if (int dim_err = validate_dimensions(rows, cols); dim_err != MX_OK) {
        return dim_err;
    }

    try {
        auto handle = std::make_unique<matrix_handle_t>();
        handle->matrix = std::make_unique<DenseMatrix>(rows, cols, data);
        *out = handle.release();
        return MX_OK;
    } catch (const shape_error&) {
        return MX_ERR_SHAPE;
    } catch (const std::bad_alloc&) {
        return MX_ERR_ALLOC;
    } catch (...) {
        return MX_ERR_UNKNOWN;
    }
}

// Release a handle previously allocated with mx_create or mx_multiply.
void mx_destroy(matrix_handle_t* h) {
    delete h;
}

// Multiply two matrix handles and return the result as a new handle.
int mx_multiply(const matrix_handle_t* a, const matrix_handle_t* b, matrix_handle_t** out) {
    if (!out) {
        return MX_ERR_NULL;
    }
    *out = nullptr;
    if (!a || !b || !a->matrix || !b->matrix) {
        return MX_ERR_STATE;
    }

    try {
        auto product = a->matrix->multiply(*b->matrix);
        auto handle = std::make_unique<matrix_handle_t>();
        handle->matrix = std::move(product);
        *out = handle.release();
        return MX_OK;
    } catch (const shape_error&) {
        return MX_ERR_SHAPE;
    } catch (const std::bad_alloc&) {
        return MX_ERR_ALLOC;
    } catch (...) {
        return MX_ERR_UNKNOWN;
    }
}

// Read the number of rows stored in a handle, returning 0 for invalid inputs.
size_t mx_rows(const matrix_handle_t* h) {
    if (!h || !h->matrix) {
        return 0;
    }
    return h->matrix->rows();
}

// Read the number of columns stored in a handle, returning 0 for invalid inputs.
size_t mx_cols(const matrix_handle_t* h) {
    if (!h || !h->matrix) {
        return 0;
    }
    return h->matrix->cols();
}

// Copy matrix contents into caller-supplied memory.
int mx_copy_out(const matrix_handle_t* h, double* out, size_t out_len) {
    if (!h || !h->matrix) {
        return MX_ERR_STATE;
    }
    if (!out) {
        return MX_ERR_NULL;
    }
    const std::size_t rows = h->matrix->rows();
    const std::size_t cols = h->matrix->cols();
    if (out_len != rows * cols) {
        return MX_ERR_SHAPE;
    }
    const double* src = h->matrix->data();
    std::copy(src, src + out_len, out);
    return MX_OK;
}

