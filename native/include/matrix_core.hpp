#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#if defined(__GNUC__) || defined(__clang__)
#define MX_RESTRICT __restrict__
#else
#define MX_RESTRICT
#endif

namespace matrix {

// Exception thrown when matrix dimensions are incompatible for the requested
// operation (for example, multiplication of mismatched shapes).
class shape_error : public std::runtime_error {
public:
    explicit shape_error(const char* msg) : std::runtime_error(msg) {}
};

// Dense, row-major matrix of double-precision values. The class owns the
// backing storage and provides a high-performance general matrix
// multiplication (GEMM) routine for row-major inputs.
class DenseMatrix {
public:
    // Construct an empty matrix with the given shape.
    DenseMatrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols) {
        if (rows_ == 0 || cols_ == 0) throw shape_error("rows and cols must be > 0");
    }

    // Construct a matrix from a row-major buffer.
    DenseMatrix(std::size_t rows, std::size_t cols, const double* src)
        : DenseMatrix(rows, cols) {
        if (!src) throw std::invalid_argument("src is null");
        std::copy(src, src + rows * cols, data_.begin());
    }

    // Number of rows in the matrix.
    std::size_t rows() const noexcept { return rows_; }
    // Number of columns in the matrix.
    std::size_t cols() const noexcept { return cols_; }
    // Pointer to the underlying data (read-only view).
    const double* data() const noexcept { return data_.data(); }
    // Pointer to the underlying data (mutable view).
    double* data() noexcept { return data_.data(); }

    // High-performance GEMM: C = A * B (row-major). The result is returned as a
    // newly allocated matrix so the operands remain unchanged.
    std::unique_ptr<DenseMatrix> multiply(const DenseMatrix& rhs) const {
        if (cols_ != rhs.rows_) throw shape_error("incompatible shapes for multiplication");

        const std::size_t M = rows_;
        const std::size_t K = cols_;
        const std::size_t N = rhs.cols_;

        auto out = std::make_unique<DenseMatrix>(M, N);
        double* MX_RESTRICT C = out->data();
        const double* MX_RESTRICT A = data();
        const double* MX_RESTRICT B = rhs.data();

        // Small sizes: straightforward triple-loop is faster.
        if (M * N * K <= 64ull * 64ull * 64ull) {
            std::fill(C, C + M * N, 0.0);
            for (std::size_t i = 0; i < M; ++i) {
                const std::size_t cRow = i * N;
                const std::size_t aRow = i * K;
                for (std::size_t k = 0; k < K; ++k) {
                    const double aik = A[aRow + k];
                    const std::size_t bRow = k * N;
                    for (std::size_t j = 0; j < N; ++j) {
                        C[cRow + j] += aik * B[bRow + j];
                    }
                }
            }
            return out;
        }

        // Transpose B into BT (shape N x K) so that BT rows are contiguous.
        std::vector<double> BT;
        BT.resize(N * K);
        for (std::size_t k = 0; k < K; ++k) {
            const double* MX_RESTRICT bRow = B + k * N;
            for (std::size_t j = 0; j < N; ++j) {
                BT[j * K + k] = bRow[j];
            }
        }

        // 3-level blocking tuned for AVX2-sized caches; adjust if needed.
        const std::size_t MC = 256;
        const std::size_t NC = 256;
        const std::size_t KC = 256;

        std::fill(C, C + M * N, 0.0);

        // Parallelize over (i0, j0) tiles; k0 stays inside to reuse packed/BT blocks.
        #pragma omp parallel for collapse(2) schedule(static)
        for (std::ptrdiff_t i0 = 0; i0 < static_cast<std::ptrdiff_t>(M); i0 += MC) {
            for (std::ptrdiff_t j0 = 0; j0 < static_cast<std::ptrdiff_t>(N); j0 += NC) {

                const std::size_t iMax = static_cast<std::size_t>(std::min<std::ptrdiff_t>(i0 + MC, M));
                const std::size_t jMax = static_cast<std::size_t>(std::min<std::ptrdiff_t>(j0 + NC, N));

                for (std::size_t k0 = 0; k0 < K; k0 += KC) {
                    const std::size_t kMax = std::min(k0 + KC, K);
                    const std::size_t kBlock = kMax - k0;

                    for (std::size_t i = static_cast<std::size_t>(i0); i < iMax; ++i) {
                        const double* MX_RESTRICT aBlock = A + i * K + k0;
                        double* MX_RESTRICT cRow = C + i * N + j0;

                        for (std::size_t j = static_cast<std::size_t>(j0); j < jMax; ++j) {
                            const double* MX_RESTRICT btRow = BT.data() + j * K + k0;

                            double acc = cRow[j - j0];

                            // Manually unrolled dot product (4x).
                            std::size_t kk = 0;
                            for (; kk + 3 < kBlock; kk += 4) {
                                acc += aBlock[kk + 0] * btRow[kk + 0];
                                acc += aBlock[kk + 1] * btRow[kk + 1];
                                acc += aBlock[kk + 2] * btRow[kk + 2];
                                acc += aBlock[kk + 3] * btRow[kk + 3];
                            }
                            for (; kk < kBlock; ++kk) {
                                acc += aBlock[kk] * btRow[kk];
                            }

                            cRow[j - j0] = acc;
                        }
                    }
                }
            }
        }

        return out;
    }

private:
    // Dimensions of the matrix in row-major order.
    std::size_t rows_;
    std::size_t cols_;
    // Contiguous storage for rows_ * cols_ double values.
    std::vector<double> data_;
};

} // namespace matrix
