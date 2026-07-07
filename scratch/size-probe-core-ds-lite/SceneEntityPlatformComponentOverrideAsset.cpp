#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "runtime/array.hpp"

SceneEntityPlatformComponentOverrideAsset::SceneEntityPlatformComponentOverrideAsset() : PlatformId(), RemovedComponentKeys(Array<std::string>::Empty()), AddedComponents(Array<SceneEntityPlatformAddedComponentAsset*>::Empty())
{
}

const std::string& SceneEntityPlatformComponentOverrideAsset::get_PlatformId()
{
return this->PlatformId;
}

void SceneEntityPlatformComponentOverrideAsset::set_PlatformId(std::string value)
{
this->PlatformId = value;
}

Array<std::string>* SceneEntityPlatformComponentOverrideAsset::get_RemovedComponentKeys()
{
return this->RemovedComponentKeys;
}

void SceneEntityPlatformComponentOverrideAsset::set_RemovedComponentKeys(Array<std::string>* value)
{
this->RemovedComponentKeys = value;
}

Array<::SceneEntityPlatformAddedComponentAsset*>* SceneEntityPlatformComponentOverrideAsset::get_AddedComponents()
{
return this->AddedComponents;
}

void SceneEntityPlatformComponentOverrideAsset::set_AddedComponents(Array<::SceneEntityPlatformAddedComponentAsset*>* value)
{
this->AddedComponents = value;
}

