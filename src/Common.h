#pragma once

#include <cstdint>

enum class ALGORITHM : uint8_t {
    BUBBLE_SORT, COMB_SORT, INSERT_SORT, SELECT_SORT, SHAKER_SORT,
    QUICK_SORT, MERGE_SORT, HEAP_SORT,

    SEAM_CARVING,
    // QUADTREE_DECOMPOSITION,
    // FOURIER_TRANSFORM_RECONSTRUCTION

    INVALID
};

struct Pixel {
    uint8_t r,g,b,a;
};
