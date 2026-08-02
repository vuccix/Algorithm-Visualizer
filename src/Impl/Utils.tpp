#pragma once

// #include <omp.h>

template <typename T, typename Op, typename Writer>
void Utils::convolution(const std_mdspan<const T> image, const int32_t kHeight, const int32_t kWidth, Op&& op, Writer&& writer) {
    const auto rows  = static_cast<int32_t>(image.extent(0));
    const auto cols  = static_cast<int32_t>(image.extent(1));

    const int32_t rW = kWidth  / 2;
    const int32_t rH = kHeight / 2;

    auto index = [](const int32_t id, const int32_t max) -> int32_t {
        return (id < 0) ? -id - 1 : (id >= max) ? 2 * max - id - 1 : id;
    };

    // #pragma omp parallel for collapse(2)
    for (int32_t y = 0; y < rows; ++y) {
        for (int32_t x = 0; x < cols; ++x) {
            auto accumulator = op.init();

            for (int32_t ky = -rH; ky <= rH; ++ky) {
                const int32_t iy = index(y + ky, rows);
                const int32_t ki = ky + rH;

                for (int32_t kx = -rW; kx <= rW; ++kx) {
                    const int32_t ix = index(x + kx, cols);
                    const int32_t kj = kx + rW;

                    op.accumulate(accumulator, image[iy, ix], ki, kj);
                }
            }

            writer(x, y, accumulator);
        }
    }
}
