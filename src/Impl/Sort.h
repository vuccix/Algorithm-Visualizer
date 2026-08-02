#pragma once

#include <cstdint>
#include <vector>
#include <span>

namespace Sort {

    // O(n^2)

    void bubble(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void comb(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void insert(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void select(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void shaker(std::span<int32_t> arr, std::vector<int32_t>& sequence);

    // O(n log(n))

    void quick(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void merge(std::span<int32_t> arr, std::vector<int32_t>& sequence);
    void heap(std::span<int32_t> arr, std::vector<int32_t>& sequence);

}
