#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeStartupManifest.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeStorageProfileId.hpp"
#include "PlatformInfo.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeStartupManifest.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

const std::string& RuntimeStartupManifest::get_StartupSceneId()
{
return this->StartupSceneId;
}

::RuntimeStorageProfileId* RuntimeStartupManifest::get_StorageProfileId()
{
return this->StorageProfileId;
}

::PlatformInfo* RuntimeStartupManifest::get_PlatformInfo()
{
return this->PlatformInfo;
}

RuntimeStartupManifest::RuntimeStartupManifest(std::string startupSceneId, ::RuntimeStorageProfileId* storageProfileId, ::PlatformInfo* platformInfo) : StartupSceneId(), StorageProfileId(), PlatformInfo()
{
    if (String::IsNullOrWhiteSpace(startupSceneId))
    {
throw ([&]() {
auto __ctor_arg_0000014E = "Startup scene id is required.";
auto __ctor_arg_0000014F = "startupSceneId";
return new ArgumentException(__ctor_arg_0000014E, __ctor_arg_0000014F);
})();
    }
    if (storageProfileId == nullptr)
    {
throw new ArgumentNullException("storageProfileId");
    }
    if (platformInfo == nullptr)
    {
throw new ArgumentNullException("platformInfo");
    }
this->StartupSceneId = startupSceneId;
this->StorageProfileId = storageProfileId;
this->PlatformInfo = platformInfo;
}

