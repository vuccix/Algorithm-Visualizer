#include <Image.h>
#include <filesystem>
#include <iostream>
#include <chrono>

namespace {
    bool arePathsValid(std::string_view input, std::string_view output);
    ALGORITHM getAlgo(std::string_view algoName);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Not enough arguments! <input path> <output path> <algorithm>\n";
        return EXIT_FAILURE;
    }

    if (!::arePathsValid(argv[1], argv[2])) {
        std::cerr << "Invalid paths!\n";
        return EXIT_FAILURE;
    }

    using clock      = std::chrono::steady_clock;
    const auto start = clock::now();
    // --------------------------------------------------------------

    Image image(argv[1]);
    image.setParams(::getAlgo(argv[3]), 1, 50, 80, false); // algorithm, row height (px), frame delay (ms), quality (0-100), is reversed
    image.writeWebP(argv[2]);

    // --------------------------------------------------------------
    std::cout << "frame count: " << image.getFrameCount() << '\n';
    const auto end = clock::now();
    const auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "processing took " << dur.count() << " ms\n";

    return EXIT_SUCCESS;
}

// =====================================================================================================================

namespace {

bool arePathsValid(const std::string_view input, const std::string_view output) {
    namespace fs = std::filesystem;

    try {
        const fs::path inputPath(input);
        const fs::path outputPath(output);

        if (!fs::exists(inputPath) || !fs::is_regular_file(inputPath))
            return false;

        const fs::path outputDir = outputPath.parent_path();
        if (!outputDir.empty() && (!fs::exists(outputDir) || !fs::is_directory(outputDir)))
            return false;

        if (fs::is_directory(outputPath))
            return false;

        return true;
    }
    catch (const fs::filesystem_error&) {
        return false;
    }
}

ALGORITHM getAlgo(const std::string_view algoName) {
    if (algoName == "bubble_sort")  return ALGORITHM::BUBBLE_SORT;
    if (algoName == "comb_sort")    return ALGORITHM::COMB_SORT;
    if (algoName == "insert_sort")  return ALGORITHM::INSERT_SORT;
    if (algoName == "select_sort")  return ALGORITHM::SELECT_SORT;
    if (algoName == "shaker_sort")  return ALGORITHM::SHAKER_SORT;
    if (algoName == "heap_sort")    return ALGORITHM::HEAP_SORT;
    if (algoName == "merge_sort")   return ALGORITHM::MERGE_SORT;
    if (algoName == "quick_sort")   return ALGORITHM::QUICK_SORT;
    if (algoName == "seam_carving") return ALGORITHM::SEAM_CARVING;

    std::cerr << "Unknown algorithm: " << algoName << '\n';
    std::exit(EXIT_FAILURE);
}

}
