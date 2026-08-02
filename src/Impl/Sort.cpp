#include <Impl/Sort.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>

namespace {

void helper(std::vector<int32_t>& sequence, const std::span<int32_t> id) {
    sequence.insert(sequence.end(), id.begin(), id.end());
}

void quickSort(std::vector<int32_t>& seq, std::span<int32_t> arr, int32_t low = 0, int32_t high = -69);
void mergeSort(std::vector<int32_t>& seq, std::span<int32_t> arr, int32_t left = 0, int32_t right = -69);
void heapify(std::span<int32_t> arr, int32_t n, int32_t i);

void info(const size_t maxSwaps, const int32_t height, const std::vector<int32_t>& sequence) {
    std::cout << "estimated size: " << maxSwaps * height << '\n';
    std::cout << "sequence size:  " << sequence.size() << "\n";
    const double ratio = static_cast<double>(maxSwaps*height) / static_cast<double>(sequence.size());
    std::cout << "estimate is " << std::fixed << std::setprecision(3) << ratio << " times larger than size\n";
}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sort::bubble(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * (height - 1) / 2;
    sequence.clear();
    sequence.reserve(maxSwaps * height);

    bool swapped = false;

    for (int32_t i = 0; i < height; ++i) {
        swapped = false;

        for (int32_t j = 0; j < height - 1 - i; ++j) {
            if (arr[j] > arr[j + 1]) {
                std::swap(arr[j], arr[j + 1]);
                swapped = true;
                ::helper(sequence, arr);
            }
        }

        if (!swapped) break;
    }

    // ::info(maxSwaps, height, sequence);
}

void Sort::comb(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    constexpr float shrink  = 1.3f;
    const     auto  height  = static_cast<int32_t>(arr.size());
    int32_t         gap     = height;
    bool            swapped = false;

    const size_t maxSwaps = static_cast<size_t>(height) * static_cast<size_t>(std::log(static_cast<float>(height)));
    sequence.clear();
    sequence.reserve(maxSwaps * height);

    while (gap > 1 || swapped) {
        gap = static_cast<int32_t>( static_cast<float>(gap) / shrink );
        if (gap < 1) gap = 1;

        swapped = false;

        for (int32_t i = 0; i < height - gap; ++i) {
            if (arr[i] > arr[i + gap]) {
                std::swap(arr[i], arr[i + gap]);
                swapped = true;
                ::helper(sequence, arr);
            }
        }
    }

    // ::info(maxSwaps, height, sequence);
}

void Sort::insert(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * (height - 1) / 2;
    sequence.clear();
    sequence.reserve(maxSwaps * height);

    for (int32_t i = 1; i < height; ++i) {
        const int32_t key = arr[i];
        int32_t cur       = i - 1;

        while (cur >= 0 && arr[cur] > key) {
            arr[cur + 1] = arr[cur];
            cur--;
            ::helper(sequence, arr);
        }
        arr[cur + 1] = key;
        ::helper(sequence, arr);
    }

    // ::info(maxSwaps, height, sequence);
}

void Sort::select(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * static_cast<size_t>(std::log(static_cast<float>(height)));
    sequence.clear();
    sequence.reserve(maxSwaps * height);

    for (int32_t i = 0; i < height; ++i) {
        int32_t smallest = i;

        for (int32_t j = i; j < height; ++j) {
            if (arr[j] < arr[smallest]) {
                smallest = j;
                ::helper(sequence, arr);
            }
        }

        if (smallest != i) {
            std::swap(arr[i], arr[smallest]);
            ::helper(sequence, arr);
        }
    }

    // ::info(maxSwaps, height, sequence);
}

void Sort::shaker(std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * (height - 1) / 2;
    sequence.clear();
    sequence.reserve(maxSwaps * height);

    bool sorted;

    do {
        sorted = true;

        for (int32_t i = 0; i < height - 1; ++i) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
                sorted = false;
                ::helper(sequence, arr);
            }
        }

        for (int32_t i = height - 1; i > 0; --i) {
            if (arr[i - 1] > arr[i]) {
                std::swap(arr[i - 1], arr[i]);
                sorted = false;
                ::helper(sequence, arr);
            }
        }
    } while (!sorted);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Sort::quick(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * static_cast<size_t>(std::log(static_cast<float>(height)));
    sequence.reserve(height * maxSwaps);

    ::quickSort(sequence, arr);

    // ::info(maxSwaps, height, sequence);
}

void Sort::merge(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * static_cast<size_t>(std::log(static_cast<float>(height)));
    sequence.reserve(height * maxSwaps);

    ::mergeSort(sequence, arr);

    // ::info(maxSwaps, height, sequence);
}

void Sort::heap(const std::span<int32_t> arr, std::vector<int32_t>& sequence) {
    const auto height = static_cast<int32_t>(arr.size());

    const size_t maxSwaps = static_cast<size_t>(height) * static_cast<size_t>(std::log(static_cast<float>(height)));
    sequence.reserve(height * maxSwaps);

    for (int32_t i = height / 2 - 1; i >= 0; --i) {
        ::heapify(arr, height, i);
        ::helper(sequence, arr);
    }

    for (int32_t i = height - 1; i > 0; --i) {
        std::swap(arr[0], arr[i]);
        ::heapify(arr, i, 0);
        ::helper(sequence, arr);
    }

    // ::info(maxSwaps, height, sequence);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

void merge(std::vector<int32_t>& seq, const std::span<int32_t> arr, const int32_t left, const int32_t mid, const int32_t right) {
    const int32_t n1 = mid   - left + 1;
    const int32_t n2 = right - mid;

    std::vector<int32_t> leftVec(n1), rightVec(n2);

    for (int32_t i = 0; i < n1; ++i)
        leftVec[i] = arr[left + i];
    for (int32_t j = 0; j < n2; ++j)
        rightVec[j] = arr[mid + 1 + j];

    int32_t i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftVec[i] <= rightVec[j]) {
            arr[k] = leftVec[i];
            ++i;
        }
        else {
            arr[k] = rightVec[j];
            ++j;
        }
        ++k;
    }

    while (i < n1) {
        arr[k] = leftVec[i];
        ++i;
        ++k;
    }

    while (j < n2) {
        arr[k] = rightVec[j];
        ++j;
        ++k;
    }

    ::helper(seq, arr);
}

void mergeSort(std::vector<int32_t>& seq, const std::span<int32_t> arr, const int32_t left, int32_t right) {
    const auto height = static_cast<int32_t>(arr.size());

    if (right == -69) right = height - 1;

    if (left >= right) return;

    const int32_t mid = left + (right - left) / 2;

    ::mergeSort(seq, arr, left, mid);
    ::mergeSort(seq, arr, mid + 1, right);

    ::merge(seq, arr, left, mid, right);
    ::helper(seq, arr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void heapify(const std::span<int32_t> arr, const int32_t n, const int32_t i) {
    int32_t     largest = i;
    const int32_t  left = 2 * i + 1;
    const int32_t right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        std::swap(arr[i], arr[largest]);
        ::heapify(arr, n, largest);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void quickSort(std::vector<int32_t>& seq, const std::span<int32_t> arr, const int32_t low, int32_t high) {
    auto randomRange = [](const int32_t min, const int32_t max) -> int32_t {
        thread_local static std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution dist(min, max);
        return dist(gen);
    };

    auto partition = [&](const int32_t _lo, const int32_t _hi) -> int32_t {
        const int32_t pIdx = randomRange(_lo, _hi);
        std::swap(arr[pIdx], arr[_hi]);
        ::helper(seq, arr);

        const int32_t pivot = arr[_hi];
        int32_t i = _lo - 1;

        for (int32_t j = _lo; j < _hi; ++j) {
            if (arr[j] < pivot) {
                ++i;
                std::swap(arr[i], arr[j]);
                ::helper(seq, arr);
            }
        }

        std::swap(arr[i + 1], arr[_hi]);
        ::helper(seq, arr);

        return i + 1;
    };

    const auto height = static_cast<int32_t>(arr.size());

    if (high == -69) high = height - 1;

    if (low < high) {
        const int32_t pivotIndex = partition(low, high);

        ::quickSort(seq, arr, low, pivotIndex - 1);
        ::quickSort(seq, arr, pivotIndex + 1, high);
    }
}

}
