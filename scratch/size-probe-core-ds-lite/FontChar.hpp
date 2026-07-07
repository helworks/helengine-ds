#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class float4;

#include "float4.hpp"

class FontChar
{
public:
    FontChar();

    ::float4 SourceRect;

    float OffsetY;

    float AdvanceWidth;

    float BearingX;

    float BearingY;

    FontChar(::float4 sourceRect, float offsetY, float advanceWidth, float bearingX, float bearingY);
};
