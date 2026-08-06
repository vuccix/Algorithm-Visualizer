#include <Image.h>
#include <Impl/Sort.h>
#include <Impl/Algo.h>
#include <stb_image.h>
#include <webp.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <format>
#include <random>
#include <fstream>

Image::Image(const std::string_view path) {
    int32_t channels;
    uint8_t* data = stbi_load(path.data(), &m_width, &m_height, &channels, m_channels);
    if (!data) throw std::runtime_error("Failed to load image");

    const auto total = static_cast<size_t>(m_width) * m_height;
    auto* pixelData  = reinterpret_cast<Pixel*>(data);
    m_image.assign(pixelData, pixelData + total);

    stbi_image_free(data);
}

Image::Image(const uint8_t* data, const int32_t size) {
    int32_t channels;
    uint8_t* decoded = stbi_load_from_memory(data, size, &m_width, &m_height, &channels, m_channels);
    if (!decoded) throw std::runtime_error("Failed to load image from memory");

    const auto total = static_cast<size_t>(m_width) * m_height;
    auto* pixelData  = reinterpret_cast<Pixel*>(decoded);
    m_image.assign(pixelData, pixelData + total);

    stbi_image_free(decoded);
}

void Image::writeWebP(const char* filename) {
    auto* enc = static_cast<WebPAnimEncoder*>(process());

    WebPData webpData;
    WebPDataInit(&webpData);
    WebPAnimEncoderAssemble(enc, &webpData);

    std::ofstream out(filename, std::ios::binary);
    out.write(reinterpret_cast<const char*>(webpData.bytes), static_cast<int64_t>(webpData.size));

    WebPDataClear(&webpData);
    WebPAnimEncoderDelete(enc);
}

std::vector<uint8_t> Image::generateWebP() {
    auto* enc = static_cast<WebPAnimEncoder*>(process());

    WebPData webpData;
    WebPDataInit(&webpData);
    WebPAnimEncoderAssemble(enc, &webpData);

    std::vector result(webpData.bytes, webpData.bytes + webpData.size);

    WebPDataClear(&webpData);
    WebPAnimEncoderDelete(enc);

    return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Image::setParams(const ALGORITHM algo, const int32_t rowHeight,
                      const int32_t delay_ms, const float quality, const bool reverse) noexcept {
    m_algorithm = algo;
    m_delay_ms  = delay_ms;
    m_quality   = std::clamp(quality, 0.f, 100.f);
    m_reverse   = reverse;
    m_rowHeight = std::clamp(rowHeight, 1, m_height);
}

size_t Image::getFrameCount() const noexcept {
    return m_sequence.size() / (m_height / m_rowHeight);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* Image::process() {
    assert(m_algorithm != ALGORITHM::INVALID);

    WebPAnimEncoderOptions animOptions;
    WebPAnimEncoderOptionsInit(&animOptions);

    WebPAnimEncoder* enc = WebPAnimEncoderNew(m_width, m_height, &animOptions);
    if (!enc) return nullptr;

    int32_t timestamp_ms = 0;

    WebPConfig config;
    WebPConfigInit(&config);

    config.quality          = m_quality;
    config.sns_strength     = 0;
    config.filter_sharpness = 0;
    config.exact            = 1;
    config.near_lossless    = 100;
    config.method           = 2;

    WebPPicture pic;
    if (!WebPPictureInit(&pic)) {
        WebPAnimEncoderDelete(enc);
        return nullptr;
    }

    pic.width    = m_width;
    pic.height   = m_height;
    pic.use_argb = 1;

    auto addFrame = [&](const Pixel* pixels, const int32_t strideBytes, const int32_t delay_ms) {
        if (WebPPictureImportRGBA(&pic, reinterpret_cast<const uint8_t*>(pixels), strideBytes)) {
            timestamp_ms += delay_ms;
            WebPAnimEncoderAdd(enc, &pic, timestamp_ms, &config);
        }
    };

    if (m_algorithm < ALGORITHM::SEAM_CARVING) {
        sort();
        writeSort(addFrame);
    }
    else {
        carve();
        writeCarve(addFrame);
    }

    WebPPictureFree(&pic);
    WebPAnimEncoderAdd(enc, nullptr, timestamp_ms, nullptr);

    return enc;
}

void Image::sort() {
    std::vector<int32_t> permutation(m_height / m_rowHeight);
    std::iota(permutation.begin(), permutation.end(), 0);

    std::random_device rd;
    std::mt19937 g(rd());
    std::ranges::shuffle(permutation, g);

    auto sort = [&](auto&& func) { func(permutation, m_sequence); };

    switch (m_algorithm) {
        case ALGORITHM::BUBBLE_SORT: sort(Sort::bubble); break;
        case ALGORITHM::COMB_SORT:   sort(Sort::comb);   break;
        case ALGORITHM::INSERT_SORT: sort(Sort::insert); break;
        case ALGORITHM::SELECT_SORT: sort(Sort::select); break;
        case ALGORITHM::SHAKER_SORT: sort(Sort::shaker); break;
        case ALGORITHM::QUICK_SORT:  sort(Sort::quick);  break;
        case ALGORITHM::MERGE_SORT:  sort(Sort::merge);  break;
        case ALGORITHM::HEAP_SORT:   sort(Sort::heap);   break;

        default: assert(false && "Unknown sort");
    }
}

void Image::carve() {
    m_sequence.reserve(m_height * m_width);
    Algo::seamCarving(m_image, m_width, m_height, m_sequence);
}

void Image::writeSort(const std::function<void(const Pixel*, int32_t, int32_t)>& addFrame) const {
    const int32_t numBlocks = m_height / m_rowHeight;
    const int32_t numFrames = static_cast<int32_t>(m_sequence.size()) / numBlocks;
    const int32_t rowBytes  = m_width * static_cast<int32_t>(sizeof(Pixel));

    if (numFrames == 0) return;

    std::vector<Pixel> frameBuffer(m_width * m_height);
    const std::span seqSpan(m_sequence);

    auto renderFrame = [&](const std::span<const int32_t> index, const int32_t delay_ms) {
        Pixel* dest = frameBuffer.data();

        for (int32_t y = 0; y < numBlocks; ++y) {
            const int32_t startRow  = index[y] * m_rowHeight;
            const int32_t curHeight = (y == numBlocks - 1)
                                    ? (m_height - (y * m_rowHeight))
                                    : m_rowHeight;

            for (int32_t x = 0; x < curHeight; ++x) {
                std::memcpy(dest, &m_image[(startRow + x) * m_width], rowBytes);
                dest += m_width;
            }
        }

        addFrame(frameBuffer.data(), rowBytes, delay_ms);
    };

    // first frame
    renderFrame(seqSpan.subspan(0, numBlocks), m_delay_ms * 12);

    // sorting
    for (int32_t i = 0; i < numFrames; ++i)
        renderFrame(seqSpan.subspan(i * numBlocks, numBlocks), m_delay_ms);

    // last frame
    const int32_t multiplier = m_delay_ms <= 50 ? 72 : 36;
    renderFrame(seqSpan.subspan((numFrames - 1) * numBlocks, numBlocks), multiplier * m_delay_ms);
}

void Image::writeCarve(const std::function<void(const Pixel*, int32_t, int32_t)>& addFrame) const {
    const int32_t numFrames = static_cast<int32_t>(m_sequence.size()) / m_height;
    if (numFrames <= 0) return;

    std::vector<Pixel> frameBuffer = m_image;
    const std::span seqSpan(m_sequence);

    int32_t       widthImg    = m_width;
    const int32_t strideBytes = m_width * static_cast<int32_t>(sizeof(Pixel));

    // first frame
    addFrame(frameBuffer.data(), strideBytes, m_delay_ms * 24);

    // seam carving
    for (int32_t i = 0; i < numFrames; ++i) {
        const std::span seam = seqSpan.subspan(i * m_height, m_height);

        // highlight seam
        for (int32_t y = 0; y < m_height; ++y)
            frameBuffer[y * m_width + seam[y]] = Pixel{ 255, 0, 0, 255 };

        addFrame(frameBuffer.data(), strideBytes, m_delay_ms);

        // remove seam and shift pixels
        for (int32_t y = 0; y < m_height; ++y) {
            const int32_t seam_x = seam[y];
            Pixel* row           = &frameBuffer[y * m_width];

            std::memmove(row + seam_x,
                         row + seam_x + 1,
                         (widthImg - seam_x - 1) * sizeof(Pixel));
            row[widthImg - 1] = Pixel{ 0, 0, 0, 0 };
        }

        addFrame(frameBuffer.data(), strideBytes, m_delay_ms);
        --widthImg;
    }

    // last frame
    addFrame(frameBuffer.data(), strideBytes, m_delay_ms * 12);
}
