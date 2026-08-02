#include <Impl/Utils.h>
#include <Common.h>
#include <algorithm>
#include <cassert>
#include <cmath>

template <typename T>
std::vector<T> Utils::promote(const std::span<const Pixel> pixels, const bool normalize) {
    assert(pixels.empty() == false);

    std::vector<T> result(pixels.size(), T{});

    if (normalize) {
        for (size_t i = 0; i < pixels.size(); ++i)
            result[i] = static_cast<T>(pixels[i].r) * (1.f / 255.f);
    }
    else {
        for (size_t i = 0; i < pixels.size(); ++i)
            result[i] = static_cast<T>(pixels[i].r);
    }

    return result;
}

template <typename T>
void Utils::demote(const std::span<const T> data, const std::span<Pixel> pixels) {
    assert(data.size() == pixels.size());

    constexpr T lo = 0, hi = 255;

    for (size_t i = 0; i < data.size(); ++i) {
        const auto val = static_cast<uint8_t>(std::clamp(data[i], lo, hi));
        pixels[i].r    = val;
        pixels[i].g    = val;
        pixels[i].b    = val;
    }
}

// explicit instantiation ==============================================================================================

namespace Utils {

#define INSTANTIATE_PROMOTE(T)                                        \
    template std::vector<T> promote<T>(std::span<const Pixel>, bool); \

#define INSTANTIATE_DEMOTE(T)                                         \
    template void demote<T>(std::span<const T>, std::span<Pixel>);    \

#define INSTANTIATE_CONVERSION(T)                                     \
    INSTANTIATE_PROMOTE(T)                                            \
    INSTANTIATE_DEMOTE(T)                                             \

// promote/demote
INSTANTIATE_CONVERSION(int32_t)
INSTANTIATE_CONVERSION(float)

#undef INSTANTIATE_PROMOTE
#undef INSTANTIATE_DEMOTE
#undef INSTANTIATE_CONVERSION

}

// =====================================================================================================================

Utils::SobelData Utils::getSobel(const std::span<const Pixel> image, const uint32_t width, const uint32_t height) {
    SobelData result{};
    auto& magnitude      = result.magnitude;
    auto& outX           = result.Gx;
    auto& outY           = result.Gy;

    constexpr float dx[] = { -1.f, 0.f, 1.f, /**/ -2.f, 0.f, 2.f, /**/ -1.f,  0.f,  1.f };
    constexpr float dy[] = {  1.f, 2.f, 1.f, /**/  0.f, 0.f, 0.f, /**/ -1.f, -2.f, -1.f };
    const     auto  dxK  = std::mdspan(dx, 3, 3);
    const     auto  dyK  = std::mdspan(dy, 3, 3);

    magnitude            = Utils::promote<float>(image);
    const auto  src      = std::mdspan(image.data(), height, width);
    const auto  dst      = std::mdspan(magnitude.data(), height, width);

    outX.resize(magnitude.size(), 0.f);
    outY.resize(magnitude.size(), 0.f);
    const auto dstX = std::mdspan(outX.data(), height, width);
    const auto dstY = std::mdspan(outY.data(), height, width);

    struct State { float sumX, sumY; };

    struct GradientOp {
        const std_mdspan<const float> dx;
        const std_mdspan<const float> dy;

        State init() { return { 0.f, 0.f }; }

        void accumulate(State& state, const Pixel pixel, const int32_t ki, const int32_t kj) const {
            state.sumX += pixel.r * dx[ki, kj];
            state.sumY += pixel.r * dy[ki, kj];
        }
    };

    Utils::convolution(src, 3, 3, GradientOp{ dxK, dyK },
        [&](const int32_t x, const int32_t y, const State& state) {
            dst[y, x]  = std::hypot(state.sumX, state.sumY); // std::abs(state.sumX) + std::abs(state.sumY);
            dstX[y, x] = state.sumX;
            dstY[y, x] = state.sumY;
        }
    );

    return result;
}
