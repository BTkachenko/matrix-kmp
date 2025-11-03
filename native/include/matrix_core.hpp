#include <cstdint>
#include <memory>
#include <vector>
#include <stdexcept>


namespace matrix {


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


// Cache-friendly i-k-j loop ordering.
const std::size_t M = rows_;
const std::size_t N = cols_;
const std::size_t P = rhs.cols_;
const double* A = data();
const double* B = rhs.data();
double* C = out->data();


// Zero-initialize result
std::fill(C, C + M * P, 0.0);


// Simple blocking could be added; keep it straightforward and fast enough.
for (std::size_t i = 0; i < M; ++i) {
const std::size_t arow = i * N;
const std::size_t crow = i * P;
for (std::size_t k = 0; k < N; ++k) {
const double aik = A[arow + k];
const std::size_t brow = k * P;
for (std::size_t j = 0; j < P; ++j) {
C[crow + j] += aik * B[brow + j];
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