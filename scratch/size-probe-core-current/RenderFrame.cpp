#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrame.hpp"
#include "CameraComponent.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderFrame.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::CameraComponent* RenderFrame::get_Camera()
{
return this->Camera;
}

List<::RenderFrameDrawableSubmission*>* RenderFrame::get_DrawableSubmissions()
{
return this->DrawableSubmissions;
}

List<::RenderFrameLightSubmission*>* RenderFrame::get_LightSubmissions()
{
return this->LightSubmissions;
}

List<::RenderFrameShadowCasterSubmission*>* RenderFrame::get_ShadowCasterSubmissions()
{
return this->ShadowCasterSubmissions;
}

bool RenderFrame::get_HasTransparentDrawables()
{
for (int32_t index = 0; index < this->DrawableSubmissions->get_Count(); index++) {
    if ((*this->DrawableSubmissions).get_Item(static_cast<int32_t>(index))->IsTransparent)
    {
return true;    }
}
return false;}

RenderFrame::RenderFrame(::CameraComponent* camera, List<::RenderFrameDrawableSubmission*>* drawableSubmissions, List<::RenderFrameLightSubmission*>* lightSubmissions, List<::RenderFrameShadowCasterSubmission*>* shadowCasterSubmissions) : Camera(), DrawableSubmissions(), LightSubmissions(), ShadowCasterSubmissions()
{
this->Camera = (camera != nullptr ? camera : throw new ArgumentNullException("camera"));
this->DrawableSubmissions = (drawableSubmissions != nullptr ? drawableSubmissions : throw new ArgumentNullException("drawableSubmissions"));
this->LightSubmissions = (lightSubmissions != nullptr ? lightSubmissions : throw new ArgumentNullException("lightSubmissions"));
this->ShadowCasterSubmissions = (shadowCasterSubmissions != nullptr ? shadowCasterSubmissions : throw new ArgumentNullException("shadowCasterSubmissions"));
}

