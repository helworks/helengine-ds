#ifdef DrawText
#undef DrawText
#endif
#include "SceneSettingsAsset.hpp"
#include "NativeOwnership.hpp"
#include "SceneCanvasProfile.hpp"
#include "runtime/array.hpp"
#include "SceneSettingsAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

SceneSettingsAsset::SceneSettingsAsset() : DontUnload(), CanvasProfileValue(new ::SceneCanvasProfile())
{
}

::SceneCanvasProfile* SceneSettingsAsset::get_CanvasProfile()
{
return this->CanvasProfileValue;}

void SceneSettingsAsset::set_CanvasProfile(::SceneCanvasProfile* value)
{
::SceneCanvasProfile *newValue = (value != nullptr ? value : throw new ArgumentNullException("value"));
    if (this->CanvasProfileValue != nullptr && !(this->CanvasProfileValue == newValue))
    {
delete this->CanvasProfileValue;
    }
this->CanvasProfileValue = newValue;
}

bool SceneSettingsAsset::get_DontUnload()
{
return this->DontUnload;
}

void SceneSettingsAsset::set_DontUnload(bool value)
{
this->DontUnload = value;
}

void SceneSettingsAsset::ReleaseOwnedValuesForNativeDelete()
{
delete this->CanvasProfileValue;
this->CanvasProfileValue = nullptr;
}

