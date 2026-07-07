#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class CameraComponent;
class RenderFrameDrawableSubmission;
class RenderFrameLightSubmission;
class RenderFrameShadowCasterSubmission;

#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class RenderFrame
{
public:
    virtual ~RenderFrame() = default;

    ::CameraComponent* Camera;

    ::CameraComponent* get_Camera();

    List<::RenderFrameDrawableSubmission*>* DrawableSubmissions;

    List<::RenderFrameDrawableSubmission*>* get_DrawableSubmissions();

    List<::RenderFrameLightSubmission*>* LightSubmissions;

    List<::RenderFrameLightSubmission*>* get_LightSubmissions();

    List<::RenderFrameShadowCasterSubmission*>* ShadowCasterSubmissions;

    List<::RenderFrameShadowCasterSubmission*>* get_ShadowCasterSubmissions();

    bool get_HasTransparentDrawables();

    RenderFrame(::CameraComponent* camera, List<::RenderFrameDrawableSubmission*>* drawableSubmissions, List<::RenderFrameLightSubmission*>* lightSubmissions, List<::RenderFrameShadowCasterSubmission*>* shadowCasterSubmissions);
};
