#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class FontInfo
{
public:
    virtual ~FontInfo() = default;

    std::string Name;

    const std::string& get_Name();
    void set_Name(std::string value);

    int32_t LineSpacing;

    int32_t get_LineSpacing();
    void set_LineSpacing(int32_t value);

    float SpaceWidth;

    float get_SpaceWidth();
    void set_SpaceWidth(float value);

    FontInfo(std::string name, int32_t lineSpacing, float spaceWidth);
};
