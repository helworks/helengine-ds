#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class FontAsset;

#include "runtime/native_string.hpp"
#include "TextAlignment.hpp"

class TextLayoutAlignmentUtils
{
public:
    virtual ~TextLayoutAlignmentUtils() = default;

    static double MeasureVisibleLineWidth(std::string line, ::FontAsset* font, double fontScale, double textureWidth);

    static double ResolveHorizontalOffset(::TextAlignment alignment, int32_t boxWidth, double visibleWidth);
};
