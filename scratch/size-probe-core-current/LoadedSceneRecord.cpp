#ifdef DrawText
#undef DrawText
#endif
#include "LoadedSceneRecord.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "runtime/native_exceptions.hpp"
#include "LoadedSceneRecord.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"

const std::string& LoadedSceneRecord::get_SceneId()
{
return this->SceneId;
}

const std::string& LoadedSceneRecord::get_CookedRelativePath()
{
return this->CookedRelativePath;
}

List<::Entity*>* LoadedSceneRecord::get_RootEntities()
{
return this->RootEntities;
}

::RuntimeSceneOwnedAssetSet* LoadedSceneRecord::get_OwnedAssets()
{
return this->OwnedAssets;
}

bool LoadedSceneRecord::get_DontUnload()
{
return this->DontUnload;
}

LoadedSceneRecord::LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, ::RuntimeSceneOwnedAssetSet* ownedAssets, bool dontUnload) : SceneId(), CookedRelativePath(), RootEntities(), OwnedAssets(), DontUnload()
{
    if (String::IsNullOrWhiteSpace(sceneId))
    {
throw ([&]() {
auto __ctor_arg_000000F6 = "Scene id is required.";
auto __ctor_arg_000000F7 = "sceneId";
return new ArgumentException(__ctor_arg_000000F6, __ctor_arg_000000F7);
})();
    }
    if (String::IsNullOrWhiteSpace(cookedRelativePath))
    {
throw ([&]() {
auto __ctor_arg_000000F8 = "Cooked relative path is required.";
auto __ctor_arg_000000F9 = "cookedRelativePath";
return new ArgumentException(__ctor_arg_000000F8, __ctor_arg_000000F9);
})();
    }
    if (rootEntities == nullptr)
    {
throw new ArgumentNullException("rootEntities");
    }
    if (ownedAssets == nullptr)
    {
throw new ArgumentNullException("ownedAssets");
    }
this->SceneId = sceneId;
this->CookedRelativePath = cookedRelativePath;
this->RootEntities = rootEntities;
this->OwnedAssets = ownedAssets;
this->DontUnload = dontUnload;
}

