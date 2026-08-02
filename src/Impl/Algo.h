#pragma once

#include <cstdint>
#include <vector>
#include <span>

struct Pixel;

namespace Algo {

    void seamCarving(std::span<const Pixel> pixels, int32_t width, int32_t height, std::vector<int32_t>& sequence);

}
