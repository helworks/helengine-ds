#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeTexture;

#include "RuntimeTexture.hpp"
#include "runtime/native_disposable.hpp"

class RenderTarget : public ::RuntimeTexture
{
public:
    virtual ~RenderTarget() = default;

    RenderTarget();

    bool CanSampleAsTexture;

    bool get_CanSampleAsTexture();
    void set_CanSampleAsTexture(bool value);

    bool HasDepthBuffer;

    bool get_HasDepthBuffer();
    void set_HasDepthBuffer(bool value);
};
