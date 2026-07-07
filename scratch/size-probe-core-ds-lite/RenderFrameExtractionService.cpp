#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameExtractionService.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderFrameDrawableClassifier.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrameLightClassifier.hpp"
#include "runtime/array.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"
#include "IDrawable3D.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "RenderFrame.hpp"
#include "CameraComponent.hpp"
#include "LightComponent.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeSubmesh.hpp"
#include "RuntimeModel.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "Entity.hpp"
#include "LightType.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"

::RenderFrameExtractionResult* RenderFrameExtractionService::Extract(List<::CameraComponent*>* cameras, List<::IDrawable3D*>* drawables, List<::LightComponent*>* lights, ::RendererBackendCapabilityProfile* backendCapabilities)
{
    if (cameras == nullptr)
    {
throw new ArgumentNullException("cameras");
    }
else {
    if (drawables == nullptr)
    {
throw new ArgumentNullException("drawables");
    }
else {
    if (lights == nullptr)
    {
throw new ArgumentNullException("lights");
    }
else {
    if (backendCapabilities == nullptr)
    {
throw new ArgumentNullException("backendCapabilities");
    }
}
}
}
::RenderFrameDrawableClassifier *classifier = new ::RenderFrameDrawableClassifier();
auto __localDeleteGuard_0000011B = he_cpp_make_scope_exit([&]() {
delete classifier;
});
List<::RenderFrameDrawableSubmission*> *drawableSubmissions = new List<::RenderFrameDrawableSubmission*>(static_cast<int32_t>(drawables->get_Count()));
auto __localDeleteGuard_0000011C = he_cpp_make_scope_exit([&]() {
delete drawableSubmissions;
});
List<::RenderFrameShadowCasterSubmission*> *shadowCasterSubmissions = new List<::RenderFrameShadowCasterSubmission*>(static_cast<int32_t>(drawables->get_Count()));
auto __localDeleteGuard_0000011D = he_cpp_make_scope_exit([&]() {
delete shadowCasterSubmissions;
});
for (int32_t drawableIndex = 0; drawableIndex < drawables->get_Count(); drawableIndex++) {
::IDrawable3D *drawable = (*drawables).get_Item(static_cast<int32_t>(drawableIndex));
Array<::RenderFrameDrawableSubmission*> *submissions = classifier->Classify(drawable);
for (int32_t submissionIndex = 0; submissionIndex < submissions->get_Length(); submissionIndex++) {
::RenderFrameDrawableSubmission *submission = (*submissions)[submissionIndex];
drawableSubmissions->Add(submission);
    if (!submission->IsTransparent)
    {
shadowCasterSubmissions->Add(new ::RenderFrameShadowCasterSubmission(submission->Drawable, static_cast<int32_t>(submission->SubmeshIndex), submission->Material));
    }
}
}
::RenderFrameLightClassifier *lightClassifier = new ::RenderFrameLightClassifier();
auto __localDeleteGuard_0000011E = he_cpp_make_scope_exit([&]() {
delete lightClassifier;
});
Array<::RenderFrameLightSubmission*> *lightSubmissions = new Array<RenderFrameLightSubmission*>(lights->get_Count());
for (int32_t lightIndex = 0; lightIndex < lights->get_Count(); lightIndex++) {
(*lightSubmissions)[lightIndex] = lightClassifier->Classify((*lights).get_Item(static_cast<int32_t>(lightIndex)));
}
Array<::RenderFrame*> *frames = new Array<RenderFrame*>(cameras->get_Count());
for (int32_t index = 0; index < cameras->get_Count(); index++) {
(*frames)[index] = ([&]() {
auto __ctor_arg_0000011F = (*cameras).get_Item(static_cast<int32_t>(index));
auto __ctor_arg_00000120 = new List<RenderFrameDrawableSubmission*>(drawableSubmissions->ToArray());
auto __ctor_arg_00000121 = new List<RenderFrameLightSubmission*>(lightSubmissions);
auto __ctor_arg_00000122 = new List<RenderFrameShadowCasterSubmission*>(shadowCasterSubmissions->ToArray());
return new ::RenderFrame(__ctor_arg_0000011F, __ctor_arg_00000120, __ctor_arg_00000121, __ctor_arg_00000122);
})();
}
return new ::RenderFrameExtractionResult(new List<RenderFrame*>(frames), backendCapabilities);}

