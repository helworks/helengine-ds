#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class SceneEntityPlatformExistenceOverrideAsset
{
public:
    virtual ~SceneEntityPlatformExistenceOverrideAsset() = default;

    SceneEntityPlatformExistenceOverrideAsset();

    std::string PlatformId;

    const std::string& get_PlatformId();
    void set_PlatformId(std::string value);

    bool Exists;

    bool get_Exists();
    void set_Exists(bool value);
};
