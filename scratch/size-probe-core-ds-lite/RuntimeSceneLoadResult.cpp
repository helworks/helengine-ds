#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeSceneLoadResult.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "RuntimeSceneOwnedAssetSet.hpp"
#include "runtime/native_exceptions.hpp"
#include "RuntimeSceneLoadResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

List<::Entity*>* RuntimeSceneLoadResult::get_RootEntities()
{
return this->RootEntities;
}

::RuntimeSceneOwnedAssetSet* RuntimeSceneLoadResult::get_OwnedAssets()
{
return this->OwnedAssets;
}

RuntimeSceneLoadResult::RuntimeSceneLoadResult(List<::Entity*>* rootEntities, ::RuntimeSceneOwnedAssetSet* ownedAssets) : RootEntities(), OwnedAssets()
{
this->RootEntities = (rootEntities != nullptr ? rootEntities : throw new ArgumentNullException("rootEntities"));
this->OwnedAssets = (ownedAssets != nullptr ? ownedAssets : throw new ArgumentNullException("ownedAssets"));
}

