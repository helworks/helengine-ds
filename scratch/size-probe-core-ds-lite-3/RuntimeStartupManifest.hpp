#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeStorageProfileId;
class PlatformInfo;

#include "runtime/native_string.hpp"

class RuntimeStartupManifest
{
public:
    virtual ~RuntimeStartupManifest() = default;

    std::string StartupSceneId;

    const std::string& get_StartupSceneId();

    ::RuntimeStorageProfileId* StorageProfileId;

    ::RuntimeStorageProfileId* get_StorageProfileId();

    ::PlatformInfo* PlatformInfo;

    ::PlatformInfo* get_PlatformInfo();

    RuntimeStartupManifest(std::string startupSceneId, ::RuntimeStorageProfileId* storageProfileId, ::PlatformInfo* platformInfo);
};
