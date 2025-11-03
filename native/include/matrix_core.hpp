#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace matrix {

namespace detail {

constexpr std::size_t kStrassenThreshold = 64;

inline std::size_t next_power_of_two(std::size_t value) {
    if (value <= 1) {
        return 1;
    }
    std::size_t power = 1;
    while (power < value) {
        if (power > (std::numeric_limits<std::size_t>::max() >> 1)) {
            return value;
        }
        power <<= 1;
    }
    return power;
}

inline void matrix_copy(const double* src, std::size_t src_stride,
                        double* dst, std::size_t dst_stride,
                        std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        const double* srow = src + i * src_stride;
        double* drow = dst + i * dst_stride;
        std::copy(srow, srow + n, drow);
    }
}

inline void matrix_add(const double* a, std::size_t a_stride,
                       const double* b, std::size_t b_stride,
                       double* out, std::size_t out_stride,
                       std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        const double* arow = a + i * a_stride;
        const double* brow = b + i * b_stride;
        double* orow = out + i * out_stride;
        for (std::size_t j = 0; j < n; ++j) {
            orow[j] = arow[j] + brow[j];
        }
    }
}

inline void matrix_sub(const double* a, std::size_t a_stride,
                       const double* b, std::size_t b_stride,
                       double* out, std::size_t out_stride,
                       std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        const double* arow = a + i * a_stride;
        const double* brow = b + i * b_stride;
        double* orow = out + i * out_stride;
        for (std::size_t j = 0; j < n; ++j) {
            orow[j] = arow[j] - brow[j];
        }
    }
}

inline void matrix_add_inplace(double* dst, std::size_t dst_stride,
                               const double* src, std::size_t src_stride,
                               std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        double* drow = dst + i * dst_stride;
        const double* srow = src + i * src_stride;
        for (std::size_t j = 0; j < n; ++j) {
            drow[j] += srow[j];
        }
    }
}

inline void matrix_sub_inplace(double* dst, std::size_t dst_stride,
                               const double* src, std::size_t src_stride,
                               std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        double* drow = dst + i * dst_stride;
        const double* srow = src + i * src_stride;
        for (std::size_t j = 0; j < n; ++j) {
            drow[j] -= srow[j];
        }
    }
}

inline void naive_multiply(const double* a, std::size_t a_stride,
                           const double* b, std::size_t b_stride,
                           double* c, std::size_t c_stride,
                           std::size_t n) {
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            double sum = 0.0;
            for (std::size_t k = 0; k < n; ++k) {
                sum += a[i * a_stride + k] * b[k * b_stride + j];
            }
            c[i * c_stride + j] = sum;
        }
    }
}

inline void strassen_recursive(const double* a, std::size_t a_stride,
                               const double* b, std::size_t b_stride,
                               double* c, std::size_t c_stride,
                               std::size_t n) {
    if (n <= kStrassenThreshold) {
        naive_multiply(a, a_stride, b, b_stride, c, c_stride, n);
        return;
    }

    const std::size_t k = n / 2;
    const std::size_t block = k * k;

    const double* a11 = a;
    const double* a12 = a + k;
    const double* a21 = a + k * a_stride;
    const double* a22 = a21 + k;

    const double* b11 = b;
    const double* b12 = b + k;
    const double* b21 = b + k * b_stride;
    const double* b22 = b21 + k;

    double* c11 = c;
    double* c12 = c + k;
    double* c21 = c + k * c_stride;
    double* c22 = c21 + k;

    std::vector<double> temp_a(block);
    std::vector<double> temp_b(block);

    std::vector<double> m1(block);
    std::vector<double> m2(block);
    std::vector<double> m3(block);
    std::vector<double> m4(block);
    std::vector<double> m5(block);
    std::vector<double> m6(block);
    std::vector<double> m7(block);

    // M1 = (A11 + A22) * (B11 + B22)
    matrix_add(a11, a_stride, a22, a_stride, temp_a.data(), k, k);
    matrix_add(b11, b_stride, b22, b_stride, temp_b.data(), k, k);
    strassen_recursive(temp_a.data(), k, temp_b.data(), k, m1.data(), k, k);

    // M2 = (A21 + A22) * B11
    matrix_add(a21, a_stride, a22, a_stride, temp_a.data(), k, k);
    strassen_recursive(temp_a.data(), k, b11, b_stride, m2.data(), k, k);

    // M3 = A11 * (B12 - B22)
    matrix_sub(b12, b_stride, b22, b_stride, temp_b.data(), k, k);
    strassen_recursive(a11, a_stride, temp_b.data(), k, m3.data(), k, k);

    // M4 = A22 * (B21 - B11)
    matrix_sub(b21, b_stride, b11, b_stride, temp_b.data(), k, k);
    strassen_recursive(a22, a_stride, temp_b.data(), k, m4.data(), k, k);

    // M5 = (A11 + A12) * B22
    matrix_add(a11, a_stride, a12, a_stride, temp_a.data(), k, k);
    strassen_recursive(temp_a.data(), k, b22, b_stride, m5.data(), k, k);

    // M6 = (A21 - A11) * (B11 + B12)
    matrix_sub(a21, a_stride, a11, a_stride, temp_a.data(), k, k);
    matrix_add(b11, b_stride, b12, b_stride, temp_b.data(), k, k);
    strassen_recursive(temp_a.data(), k, temp_b.data(), k, m6.data(), k, k);

    // M7 = (A12 - A22) * (B21 + B22)
    matrix_sub(a12, a_stride, a22, a_stride, temp_a.data(), k, k);
    matrix_add(b21, b_stride, b22, b_stride, temp_b.data(), k, k);
    strassen_recursive(temp_a.data(), k, temp_b.data(), k, m7.data(), k, k);

    // C11 = M1 + M4 - M5 + M7
    matrix_copy(m1.data(), k, c11, c_stride, k);
    matrix_add_inplace(c11, c_stride, m4.data(), k, k);
    matrix_sub_inplace(c11, c_stride, m5.data(), k, k);
    matrix_add_inplace(c11, c_stride, m7.data(), k, k);

    // C12 = M3 + M5
    matrix_copy(m3.data(), k, c12, c_stride, k);
    matrix_add_inplace(c12, c_stride, m5.data(), k, k);

    // C21 = M2 + M4
    matrix_copy(m2.data(), k, c21, c_stride, k);
    matrix_add_inplace(c21, c_stride, m4.data(), k, k);

    // C22 = M1 - M2 + M3 + M6
    matrix_copy(m1.data(), k, c22, c_stride, k);
    matrix_sub_inplace(c22, c_stride, m2.data(), k, k);
    matrix_add_inplace(c22, c_stride, m3.data(), k, k);
    matrix_add_inplace(c22, c_stride, m6.data(), k, k);
}

inline void strassen_multiply(const double* a, std::size_t a_stride,
                              const double* b, std::size_t b_stride,
                              double* c, std::size_t c_stride,
                              std::size_t n) {
    if (n == 0) {
        return;
    }
    strassen_recursive(a, a_stride, b, b_stride, c, c_stride, n);
}

} // namespace detail

// Exception type for internal use; we map to error codes at C API / JNI boundaries.
class shape_error : public std::runtime_error {
public:
    explicit shape_error(const char* msg) : std::runtime_error(msg) {}
};

// Simple owning dense matrix. Row-major storage.
class DenseMatrix {
public:
    DenseMatrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols) {
        if (rows_ == 0 || cols_ == 0) {
            throw shape_error("rows and cols must be > 0");
        }
    }

    DenseMatrix(std::size_t rows, std::size_t cols, const double* src)
        : DenseMatrix(rows, cols) {
        if (!src) {
            throw std::invalid_argument("src is null");
        }
        std::copy(src, src + rows * cols, data_.begin());
    }

    std::size_t rows() const noexcept { return rows_; }
    std::size_t cols() const noexcept { return cols_; }
    const double* data() const noexcept { return data_.data(); }
    double* data() noexcept { return data_.data(); }

    // C = A * B (this * rhs)
    std::unique_ptr<DenseMatrix> multiply(const DenseMatrix& rhs) const {
        if (cols_ != rhs.rows_) {
            throw shape_error("incompatible shapes for multiplication");
        }
        auto out = std::make_unique<DenseMatrix>(rows_, rhs.cols_);

        const std::size_t result_rows = rows_;
        const std::size_t inner = cols_;
        const std::size_t result_cols = rhs.cols_;

        std::size_t max_dim = std::max(result_rows, inner);
        max_dim = std::max(max_dim, result_cols);
        const std::size_t padded = detail::next_power_of_two(max_dim);

        if ((padded & (padded - 1)) != 0) {
            // Padding would overflow; fall back to the straightforward cubic algorithm.
            double* c = out->data();
            std::fill(c, c + result_rows * result_cols, 0.0);
            const double* a_ptr = data();
            const double* b_ptr = rhs.data();
            for (std::size_t i = 0; i < result_rows; ++i) {
                for (std::size_t k = 0; k < inner; ++k) {
                    const double aik = a_ptr[i * inner + k];
                    const std::size_t brow = k * result_cols;
                    const std::size_t crow = i * result_cols;
                    for (std::size_t j = 0; j < result_cols; ++j) {
                        c[crow + j] += aik * b_ptr[brow + j];
                    }
                }
            }
            return out;
        }

        std::vector<double> a_pad(padded * padded, 0.0);
        std::vector<double> b_pad(padded * padded, 0.0);
        std::vector<double> c_pad(padded * padded, 0.0);

        const double* a_src = data();
        for (std::size_t i = 0; i < result_rows; ++i) {
            std::copy(a_src + i * inner, a_src + (i + 1) * inner,
                      a_pad.data() + i * padded);
        }

        const double* b_src = rhs.data();
        for (std::size_t i = 0; i < inner; ++i) {
            std::copy(b_src + i * result_cols, b_src + (i + 1) * result_cols,
                      b_pad.data() + i * padded);
        }

        detail::strassen_multiply(a_pad.data(), padded,
                                  b_pad.data(), padded,
                                  c_pad.data(), padded,
                                  padded);

        double* c_dst = out->data();
        for (std::size_t i = 0; i < result_rows; ++i) {
            std::copy(c_pad.data() + i * padded,
                      c_pad.data() + i * padded + result_cols,
                      c_dst + i * result_cols);
        }

        return out;
    }

private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<double> data_;
};

} // namespace matrix

