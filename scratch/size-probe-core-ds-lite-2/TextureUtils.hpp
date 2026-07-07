#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeTexture;

class TextureUtils
{
public:
    virtual ~TextureUtils() = default;

    static ::RuntimeTexture* get_PixelTexture();

    static ::RuntimeTexture* get_BlackPixelTexture();
private:
    static ::RuntimeTexture* pixelTexture;

    static ::RuntimeTexture* blackPixelTexture;

    static ::RuntimeTexture* BuildSolidPixelTexture(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
};
