#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class int2;
class RuntimeMaterial;
class ContentManager;
class RuntimeModel;
class ModelAsset;
class RenderTarget;
class RendererBackendCapabilityProfile;

#include "runtime/native_disposable.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"

class RenderManager3D : public ::IDisposable
{
public:
    virtual ~RenderManager3D() = default;

    RenderManager3D();

    ::int2 MainWindowSize;

    ::int2 get_MainWindowSize();
    void set_MainWindowSize(::int2 value);

    ::Event WindowResized;

    int32_t get_LastDrawCallCount();

    virtual void AddWindow(intptr_t handle, int32_t width, int32_t height);

    virtual ::RuntimeMaterial* BuildMaterialFromRawAsset(::ContentManager* assetContentManager, std::string contentRootPath, std::string materialAssetPath);

    virtual ::RuntimeModel* BuildModelFromCooked(std::string cookedAssetPath);

    virtual ::RuntimeModel* BuildModelFromRaw(::ModelAsset* data) = 0;

    virtual ::RenderTarget* CreateRenderTarget(int32_t width, int32_t height);

    virtual void Dispose();

    virtual void Draw();

    virtual void FlushReleasedAssets();

    virtual ::RendererBackendCapabilityProfile* GetCapabilityProfile();

    virtual void OnWindowResize(intptr_t handle, int32_t newWidth, int32_t newHeight);

    virtual void ReleaseMaterial(::RuntimeMaterial* material);

    virtual void ReleaseModel(::RuntimeModel* model);

    virtual void Update();
private:
    bool setOneWindow;
};
