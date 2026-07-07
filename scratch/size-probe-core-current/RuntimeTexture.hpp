#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeData;

#include "RuntimeData.hpp"
#include "runtime/native_disposable.hpp"

class RuntimeTexture : public ::RuntimeData, public ::IDisposable
{
public:
    virtual ~RuntimeTexture() = default;

    RuntimeTexture();

    bool IsDisposed;

    bool get_IsDisposed();
    void set_IsDisposed(bool value);

    int32_t Width;

    int32_t get_Width();
    void set_Width(int32_t value);

    int32_t Height;

    int32_t get_Height();
    void set_Height(int32_t value);

    bool IsEngineOwned;

    bool get_IsEngineOwned();
    void set_IsEngineOwned(bool value);

    virtual void Dispose();
};
