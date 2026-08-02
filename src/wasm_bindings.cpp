#include <Image.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

Image* createImageFromJS(const val& js_array) {
    std::vector<uint8_t> buffer = vecFromJSArray<uint8_t>(js_array);
    return new Image(buffer.data(), buffer.size());
}

val generateWebPToJS(Image& img) {
    std::vector<uint8_t> buffer = img.generateWebP();
    val js_array                = val::global("Uint8Array").new_(buffer.size());

    js_array.call<void>("set", val(typed_memory_view(buffer.size(), buffer.data())));

    return js_array;
}

EMSCRIPTEN_BINDINGS(myModule) {
    enum_<ALGORITHM>("ALGORITHM")
        .value("BUBBLE_SORT",      ALGORITHM::BUBBLE_SORT)
        .value("COMB_SORT",        ALGORITHM::COMB_SORT)
        .value("INSERT_SORT",      ALGORITHM::INSERT_SORT)
        .value("SELECT_SORT",      ALGORITHM::SELECT_SORT)
        .value("SHAKER_SORT",      ALGORITHM::SHAKER_SORT)
        .value("QUICK_SORT",       ALGORITHM::QUICK_SORT)
        .value("MERGE_SORT",       ALGORITHM::MERGE_SORT)
        .value("HEAP_SORT",        ALGORITHM::HEAP_SORT)
        .value("SEAM_CARVING",     ALGORITHM::SEAM_CARVING);

    class_<Image>("Image")
        .function("setParams",     &Image::setParams)
        .function("getFrameCount", &Image::getFrameCount)
        .function("generateWebP",  &generateWebPToJS);

    function("createImage", &createImageFromJS, allow_raw_pointers());
}
