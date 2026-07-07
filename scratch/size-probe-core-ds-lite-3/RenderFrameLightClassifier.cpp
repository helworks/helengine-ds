#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameLightClassifier.hpp"
#include "runtime/native_exceptions.hpp"
#include "LightComponent.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "LightType.hpp"
#include "float4.hpp"
#include "ShadowMapMode.hpp"
#include "Entity.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/native_exceptions.hpp"

::RenderFrameLightSubmission* RenderFrameLightClassifier::Classify(::LightComponent* light)
{
    if (light == nullptr)
    {
throw new ArgumentNullException("light");
    }
int32_t importance = 0;
    if (light->ShadowsEnabled)
    {
importance += 1000;
    }
importance += static_cast<int32_t>(Math::Round(light->Intensity * 100.0, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero)));
return new ::RenderFrameLightSubmission(light, static_cast<int32_t>(importance));}

