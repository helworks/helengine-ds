#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderOrder2D
{
public:
    virtual ~RenderOrder2D() = default;

    inline static const uint8_t PanelBackground = 16;

    inline static const uint8_t PanelSurface = 32;

    inline static const uint8_t PanelForeground = 48;

    inline static const uint8_t PanelInteractive = 64;

    inline static const uint8_t FloatingPanelBias = 32;

    inline static const uint8_t OverlayBackground = 160;

    inline static const uint8_t OverlayForeground = 176;

    inline static const uint8_t OverlayInput = 192;

    inline static const uint8_t ModalBackground = 224;

    inline static const uint8_t ModalForeground = 240;

    inline static const uint8_t ModalOverlayBackground = 244;

    inline static const uint8_t ModalOverlayForeground = 246;

    inline static const uint8_t ModalInput = 248;
};
