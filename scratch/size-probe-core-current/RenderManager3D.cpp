#ifdef DrawText
#undef DrawText
#endif
#include "RenderManager3D.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "NativeOwnership.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "RuntimeMaterial.hpp"
#include "ContentManager.hpp"
#include "RuntimeModel.hpp"
#include "ModelAsset.hpp"
#include "RenderTarget.hpp"
#include "runtime/array.hpp"
#include "RenderManager3D.hpp"
#include "system/action.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

RenderManager3D::RenderManager3D() : MainWindowSize(), WindowResized(), setOneWindow()
{
}

::int2 RenderManager3D::get_MainWindowSize()
{
return this->MainWindowSize;
}

void RenderManager3D::set_MainWindowSize(::int2 value)
{
this->MainWindowSize = value;
}

int32_t RenderManager3D::get_LastDrawCallCount()
{
return 0;
}

void RenderManager3D::AddWindow(intptr_t handle, int32_t width, int32_t height)
{
    if (!this->setOneWindow)
    {
this->set_MainWindowSize(::int2(static_cast<int32_t>(width), static_cast<int32_t>(height)));
    }
this->setOneWindow = true;
}

::RuntimeMaterial* RenderManager3D::BuildMaterialFromRawAsset(::ContentManager* assetContentManager, std::string contentRootPath, std::string materialAssetPath)
{
    if (assetContentManager == nullptr)
    {
throw new ArgumentNullException("assetContentManager");
    }
    if (String::IsNullOrWhiteSpace(contentRootPath))
    {
throw ([&]() {
auto __ctor_arg_00000015 = "Content root path must be provided.";
auto __ctor_arg_00000016 = "contentRootPath";
return new ArgumentException(__ctor_arg_00000015, __ctor_arg_00000016);
})();
    }
    if (String::IsNullOrWhiteSpace(materialAssetPath))
    {
throw ([&]() {
auto __ctor_arg_00000017 = "Material asset path must be provided.";
auto __ctor_arg_00000018 = "materialAssetPath";
return new ArgumentException(__ctor_arg_00000017, __ctor_arg_00000018);
})();
    }
throw new NotSupportedException("This renderer does not support material creation.");
}

::RuntimeModel* RenderManager3D::BuildModelFromCooked(std::string cookedAssetPath)
{
    if (String::IsNullOrWhiteSpace(cookedAssetPath))
    {
throw ([&]() {
auto __ctor_arg_00000019 = "Cooked model asset path must be provided.";
auto __ctor_arg_0000001A = "cookedAssetPath";
return new ArgumentException(__ctor_arg_00000019, __ctor_arg_0000001A);
})();
    }
throw new NotSupportedException("This renderer does not support platform-owned cooked model creation.");
}

::RenderTarget* RenderManager3D::CreateRenderTarget(int32_t width, int32_t height)
{
throw new NotSupportedException("This renderer does not support render target creation.");
}

void RenderManager3D::Dispose()
{
}

void RenderManager3D::Draw()
{
}

void RenderManager3D::FlushReleasedAssets()
{
}

::RendererBackendCapabilityProfile* RenderManager3D::GetCapabilityProfile()
{
return new ::RendererBackendCapabilityProfile(true, false, false, false, static_cast<int32_t>(0), static_cast<int32_t>(0));}

void RenderManager3D::OnWindowResize(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
    if (!this->setOneWindow || (this->MainWindowSize.X == 0 && this->MainWindowSize.Y == 0))
    {
this->set_MainWindowSize(::int2(static_cast<int32_t>(newWidth), static_cast<int32_t>(newHeight)));
    }
this->WindowResized.Invoke(handle, newWidth, newHeight);
}

void RenderManager3D::ReleaseMaterial(::RuntimeMaterial* material)
{
    if (material == nullptr)
    {
throw new ArgumentNullException("material");
    }
if (material != nullptr)
{
material->Dispose();
delete material;
}
}

void RenderManager3D::ReleaseModel(::RuntimeModel* model)
{
    if (model == nullptr)
    {
throw new ArgumentNullException("model");
    }
if (model != nullptr)
{
model->Dispose();
delete model;
}
}

void RenderManager3D::Update()
{
}

