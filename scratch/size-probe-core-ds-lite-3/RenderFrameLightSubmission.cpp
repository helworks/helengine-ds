#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameLightSubmission.hpp"
#include "LightComponent.hpp"
#include "LightType.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderFrameLightSubmission.hpp"
#include "runtime/native_exceptions.hpp"

::LightComponent* RenderFrameLightSubmission::get_Light()
{
return this->Light;
}

::LightType RenderFrameLightSubmission::get_LightType()
{
return this->Light->LightType;
}

int32_t RenderFrameLightSubmission::get_Importance()
{
return this->Importance;
}

RenderFrameLightSubmission::RenderFrameLightSubmission(::LightComponent* light) : Light(), Importance(0)
{
    if (light == nullptr)
    {
throw new ArgumentNullException("light");
    }
this->Light = light;
}

RenderFrameLightSubmission::RenderFrameLightSubmission(::LightComponent* light, int32_t importance) : Light(), Importance(0)
{
this->Light = (light != nullptr ? light : throw new ArgumentNullException("light"));
this->Importance = importance;
}

