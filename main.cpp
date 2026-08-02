#include <Image.h>
#include <iostream>
#include <chrono>

constexpr auto image_path  = ... // insert path to image (png/jpg)
constexpr auto output_path = R"(../output/output.webp)";

int main() {
    using clock      = std::chrono::steady_clock;
    const auto start = clock::now();
    // --------------------------------------------------------------

    Image image(image_path);
    image.setParams(ALGORITHM::SEAM_CARVING, 1, 50, 80, false); // algorithm, row height (px), frame delay (ms), quality (0-100), is reversed
    image.writeWebP(output_path);

    // --------------------------------------------------------------
    std::cout << "frame count: " << image.getFrameCount() << '\n';
    const auto end = clock::now();
    const auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "processing took " << dur.count() << " ms\n";

    return 0;
}
