#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneCanvasProfile;

class SceneSettingsAsset
{
public:
    virtual ~SceneSettingsAsset() = default;

    SceneSettingsAsset();

    ::SceneCanvasProfile* get_CanvasProfile();

    void set_CanvasProfile(::SceneCanvasProfile* value);

    bool DontUnload;

    bool get_DontUnload();
    void set_DontUnload(bool value);

    void ReleaseOwnedValuesForNativeDelete();
private:
    ::SceneCanvasProfile* CanvasProfileValue;
};
