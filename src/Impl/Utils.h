#pragma once

#include <cstdint>
#include <vector>
#include <mdspan>

struct Pixel;

namespace Utils {

    template <typename T>
    std::vector<T> promote(std::span<const Pixel> pixels, bool normalize = false);

    template <typename T>
    void demote(std::span<const T> data, std::span<Pixel> pixels);

    template <typename T>
    using std_mdspan = std::mdspan<T, std::dextents<size_t, 2>>;

    template <typename State, typename KernelType>
    struct KernelOp {
        std_mdspan<const KernelType> kernel;

        State init() const { return {}; }

        void accumulate(State& acc, auto pixel, int32_t ki, int32_t kj) const {
            if constexpr (requires { acc[0]; }) {
                acc[0] += pixel.r * kernel[ki, kj];
                acc[1] += pixel.g * kernel[ki, kj];
                acc[2] += pixel.b * kernel[ki, kj];
            }
            else {
                acc += pixel * kernel[ki, kj];
            }
        }
    };

    template <typename T, typename Op, typename Writer>
    void convolution(std_mdspan<const T> image, int32_t kHeight, int32_t kWidth, Op&& op, Writer&& writer);

    struct SobelData {
        std::vector<float> magnitude;
        std::vector<float> Gx;
        std::vector<float> Gy;
    };

    SobelData getSobel(std::span<const Pixel> image, uint32_t width, uint32_t height);

}

#include "Utils.tpp"
