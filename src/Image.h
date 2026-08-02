#pragma once

#include <Common.h>
#include <functional>
#include <cstdint>
#include <vector>
#include <string>

class Image {
public:
    explicit Image(std::string_view path);
    explicit Image(const uint8_t* data, int32_t size);

    void writeWebP(const char* filename);
    std::vector<uint8_t> generateWebP();

    void setParams(ALGORITHM algo, int32_t rowHeight, int32_t delay_ms, float quality, bool reverse) noexcept;
    size_t getFrameCount() const noexcept;

private: // internals
    std::vector<Pixel>   m_image;
    std::vector<int32_t> m_sequence;

    int32_t              m_width     = 0,
                         m_height    = 0,
                         m_channels  = 4,
                         m_frame     = 0;

private: // parameters
    int32_t              m_delay_ms  = 50,
                         m_rowHeight = 1;
    float                m_quality   = 100.f;
    ALGORITHM            m_algorithm = ALGORITHM::INVALID;
    bool                 m_reverse   = false;

private: // methods
    void* process();

    void sort();
    void writeSort(const std::function<void(const Pixel*, int32_t, int32_t)>& addFrame) const;

    void carve();
    void writeCarve(const std::function<void(const Pixel*, int32_t, int32_t)>& addFrame) const;
};
