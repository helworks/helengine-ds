#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeTexture;

#include "RuntimeTexture.hpp"
#include "runtime/native_disposable.hpp"

class ManagedRuntimeTexture : public ::RuntimeTexture
{
public:
    virtual ~ManagedRuntimeTexture() = default;

    bool get_IsDisposed();

    void set_IsDisposed(bool value);

    int32_t get_Width();

    void set_Width(int32_t value);

    int32_t get_Height();

    void set_Height(int32_t value);

    bool get_IsEngineOwned();

    void set_IsEngineOwned(bool value);

    const std::string& get_Id();

    void set_Id(std::string value);
};
