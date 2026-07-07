#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class FontAsset;

#include "runtime/native_string.hpp"

class TextLayoutUtils
{
public:
    virtual ~TextLayoutUtils() = default;

    static std::string WrapText(std::string text, ::FontAsset* font, int32_t maxWidth);
private:
    static double GetCharacterAdvanceWidth(::FontAsset* font, char character);
};
