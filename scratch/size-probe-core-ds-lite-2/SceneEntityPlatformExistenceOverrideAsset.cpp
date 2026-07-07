#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_string.hpp"

SceneEntityPlatformExistenceOverrideAsset::SceneEntityPlatformExistenceOverrideAsset() : PlatformId(String::Empty), Exists()
{
}

const std::string& SceneEntityPlatformExistenceOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void SceneEntityPlatformExistenceOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

bool SceneEntityPlatformExistenceOverrideAsset::get_Exists()
{
return this->Exists;
}

void SceneEntityPlatformExistenceOverrideAsset::set_Exists(bool value)
{
this->Exists = value;
}

