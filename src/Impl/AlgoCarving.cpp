#include <Impl/Algo.h>
#include <Common.h>
#include <Impl/Utils.h>
#include <algorithm>
#include <cstring>

namespace {

template <typename T>
using std_mdspan = std::mdspan<T, std::dextents<size_t, 2>>;

void grayscale(const std_mdspan<Pixel> pixels) {
    constexpr std::array arr = { 0.2989f, 0.5870f, 0.1140f };
    const size_t height      = pixels.extent(0);
    const size_t width       = pixels.extent(1);

    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            Pixel& pixel      = pixels[y, x];
            const float value = pixel.r * arr[0] + pixel.g * arr[1] + pixel.b * arr[2];

            pixel = {
                .r = static_cast<uint8_t>(std::clamp(value, 0.f, 255.f)),
                .g = static_cast<uint8_t>(std::clamp(value, 0.f, 255.f)),
                .b = static_cast<uint8_t>(std::clamp(value, 0.f, 255.f)),
                .a = pixel.a
            };
        }
    }
}

template <typename T>
using KernelOp = Utils::KernelOp<std::array<T, 3>, T>;

void gaussianBlur(const std_mdspan<Pixel> dst) {
    const size_t height = dst.extent(0);
    const size_t width  = dst.extent(1);

    const std::vector clone(dst.data_handle(), dst.data_handle() + dst.size());
    const std::mdspan src(clone.data(), height, width);

    constexpr std::array kernel = {
        1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
        2.f / 16.f, 4.f / 16.f, 2.f / 16.f,
        1.f / 16.f, 2.f / 16.f, 1.f / 16.f,
    };

    Utils::convolution(src, 3, 3, KernelOp{ std::mdspan(kernel.data(), 3, 3) },
        [&](const int32_t x, const int32_t y, const std::array<float, 3> sum) {
            dst[y, x] = {
                .r = static_cast<uint8_t>(std::clamp(sum[0], 0.f, 255.f)),
                .g = static_cast<uint8_t>(std::clamp(sum[1], 0.f, 255.f)),
                .b = static_cast<uint8_t>(std::clamp(sum[2], 0.f, 255.f)),
                .a = dst[y, x].a
            };
        }
    );
}

constexpr float INF = 1e20f;

struct ImageSize {
    int32_t width  = 0;
    int32_t height = 0;
    int32_t stride = 0;
};

float computeLocalEnergy(const std::span<const Pixel> image, const int32_t x, const int32_t y, const ImageSize size) {
    auto getL = [&](int32_t sampleX, int32_t sampleY) -> float {
        sampleX = std::clamp(sampleX, 0, size.width - 1);
        sampleY = std::clamp(sampleY, 0, size.height - 1);
        return image[sampleY * size.stride + sampleX].r;
    };

    const float gx = -1 * getL(x - 1, y - 1) + 1 * getL(x + 1, y - 1) +
                     -2 * getL(x - 1, y + 0) + 2 * getL(x + 1, y)     +
                     -1 * getL(x - 1, y + 1) + 1 * getL(x + 1, y + 1) ;

    const float gy = -1 * getL(x - 1, y - 1) - 2 * getL(x + 0, y - 1) +
                     -1 * getL(x + 1, y - 1) + 1 * getL(x - 1, y + 1) +
                     +2 * getL(x + 0, y + 1) + 1 * getL(x + 1, y + 1) ;

    return std::abs(gx) + std::abs(gy);
}

void getCumulative(std::span<float> cumulativeData, std::span<const float> energyData, const ImageSize size) {
    const int32_t cols   = size.width;
    const int32_t rows   = size.height;
    const int32_t stride = size.stride;

    std::copy_n(energyData.begin(), cols, cumulativeData.begin());

    for (int32_t y = 1; y < rows; ++y) {
        const float* prevRow = &cumulativeData[(y - 1) * stride];
        float*       curRow  = &cumulativeData[y * stride];
        const float* eRow    = &energyData[y * stride];

        // left boundary (x = 0)
        {
            const float c =            prevRow[0] ;
            const float r = cols > 1 ? prevRow[1] : ::INF;
            curRow[0]     =            eRow[0]    + std::min(c, r);
        }

        // middle
        for (int32_t x = 1; x < cols - 1; ++x) {
            const float l = prevRow[x - 1];
            const float c = prevRow[x + 0];
            const float r = prevRow[x + 1];
            curRow[x]     = eRow[x] + std::min(l, std::min(c, r));
        }

        // right boundary (x = cols - 1)
        if (cols > 1) {
            const int32_t x = cols - 1;
            const float   l = prevRow[x - 1];
            const float   c = prevRow[x + 0];
            curRow[x]       = eRow[x] + std::min(l, c);
        }
    }
}

void findSeam(std::span<int32_t> seam, const std::span<const float> cumulativeData, const ImageSize size) {
    const int32_t cols   = size.width;
    const int32_t rows   = size.height;
    const int32_t stride = size.stride;

    const std::mdspan cumulative(cumulativeData.data(), rows, stride);

    // find min value in bottom row
    float minVal = ::INF;
    for (int32_t x = 0; x < cols; ++x) {
        if (cumulative[rows - 1, x] < minVal) {
            seam[rows - 1] = x;
            minVal         = cumulative[rows - 1, x];
        }
    }

    // backtrack to find seam
    for (int32_t y = rows -1; y > 0; --y) {
        const int32_t seam_y = seam[y];

        const float l = (seam_y > 0)        ? cumulative[y - 1, seam_y - 1] : ::INF;
        const float c =                       cumulative[y - 1, seam_y + 0] ;
        const float r = (seam_y < cols - 1) ? cumulative[y - 1, seam_y + 1] : ::INF;

        if      (seam_y > 0 && l < c && l < r)        seam[y - 1] = seam_y - 1; // left
        else if (seam_y < cols - 1 && r < c && r < l) seam[y - 1] = seam_y + 1; // right
        else                                          seam[y - 1] = seam_y + 0; // center
    }
}

void removeSeam(const std::span<const int32_t> seam, std::span<float> energy, std::span<Pixel> image, const ImageSize size) {
    const int32_t rows   = size.height;
    const int32_t cols   = size.width;
    const int32_t stride = size.stride;

    for (int32_t y = 0; y < rows; ++y) {
        const int32_t seam_x = seam[y];

        // IMAGE
        Pixel* row = &image[y * stride];
        std::memmove(row + seam_x,
                     row + (seam_x + 1),
                     (cols - seam_x - 1) * sizeof(Pixel));

        // ENERGY
        float* e_row = &energy[y * stride];
        std::memmove(e_row + seam_x,
                     e_row + seam_x + 1,
                     (cols - seam_x - 1) * sizeof(float));

        // local energy update
        ImageSize nextSize = size;
        nextSize.width--;

        // update left pixel
        if (seam_x > 0)
            e_row[seam_x - 1] = computeLocalEnergy(image, seam_x - 1, y, nextSize);

        // update right pixel
        if (seam_x < nextSize.width)
            e_row[seam_x] = computeLocalEnergy(image, seam_x, y, nextSize);
    }
}

}

void Algo::seamCarving(const std::span<const Pixel> pixels, const int32_t width, const int32_t height, std::vector<int32_t>& sequence) {
    std::vector<Pixel> copy(pixels.begin(), pixels.end());
    const std::mdspan image(copy.data(), height, width);

    ::grayscale(image);
    ::gaussianBlur(image);

    ImageSize imgSize = {
        .width  = width,
        .height = height,
        .stride = width,
    };

    std::vector energy = Utils::getSobel(copy, imgSize.width, imgSize.height).magnitude;
    std::vector cumulative(imgSize.width * imgSize.height, 0.f);
    std::vector seam(imgSize.height, 0);

    for (int32_t i = 0; i < width; ++i) {
        ::getCumulative(cumulative, energy, imgSize);
        ::findSeam(seam, cumulative, imgSize);
        ::removeSeam(seam, energy, copy, imgSize);

        sequence.insert(sequence.end(), seam.begin(), seam.end());
        imgSize.width--;
    }
}
