#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace matrix {

class shape_error : public std::runtime_error {
public:
    explicit shape_error(const char* msg) : std::runtime_error(msg) {}
};

class DenseMatrix {
public:
    DenseMatrix(std::size_t rows, std::size_t cols)
        : rows_(rows), cols_(cols), data_(rows * cols) {
        if (rows_ == 0 || cols_ == 0) throw shape_error("rows and cols must be > 0");
    }

    DenseMatrix(std::size_t rows, std::size_t cols, const double* src)
        : DenseMatrix(rows, cols) {
        if (!src) throw std::invalid_argument("src is null");
        std::copy(src, src + rows * cols, data_.begin());
    }

    std::size_t rows() const noexcept { return rows_; }
    std::size_t cols() const noexcept { return cols_; }
    const double* data() const noexcept { return data_.data(); }
    double* data() noexcept { return data_.data(); }

    std::unique_ptr<DenseMatrix> multiply(const DenseMatrix& rhs) const {
        if (cols_ != rhs.rows_) throw shape_error("incompatible shapes for multiplication");
        auto out = std::make_unique<DenseMatrix>(rows_, rhs.cols_);

        const std::size_t M = rows_;
        const std::size_t K = cols_;
        const std::size_t N = rhs.cols_;
        double* c = out->data();
        const double* a_ptr = data();
        const double* b_ptr = rhs.data();

        std::fill(c, c + M * N, 0.0);

        const std::size_t BS = 64; // tile
        for (std::size_t i0 = 0; i0 < M; i0 += BS) {
            const std::size_t iMax = std::min(i0 + BS, M);
            for (std::size_t k0 = 0; k0 < K; k0 += BS) {
                const std::size_t kMax = std::min(k0 + BS, K);
                for (std::size_t j0 = 0; j0 < N; j0 += BS) {
                    const std::size_t jMax = std::min(j0 + BS, N);
                    for (std::size_t i = i0; i < iMax; ++i) {
                        const std::size_t crow = i * N;
                        for (std::size_t k = k0; k < kMax; ++k) {
                            const double aik = a_ptr[i * K + k];
                            const std::size_t brow = k * N;
                            for (std::size_t j = j0; j < jMax; ++j) {
                                c[crow + j] += aik * b_ptr[brow + j];
                            }
                        }
                    }
                }
            }
        }
        return out;
    }

private:
    std::size_t rows_;
    std::size_t cols_;
    std::vector<double> data_;
};

} // namespace matrix
