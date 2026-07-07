#ifdef DrawText
#undef DrawText
#endif
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneComponentAssetRecord.hpp"

SceneEntityPlatformAddedComponentAsset::SceneEntityPlatformAddedComponentAsset() : Component()
{
}

::SceneComponentAssetRecord* SceneEntityPlatformAddedComponentAsset::get_Component()
{
return this->Component;
}

void SceneEntityPlatformAddedComponentAsset::set_Component(::SceneComponentAssetRecord* value)
{
this->Component = value;
}

