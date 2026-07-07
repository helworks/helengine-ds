#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameLightSubmission;
class LightComponent;

class RenderFrameLightClassifier
{
public:
    virtual ~RenderFrameLightClassifier() = default;

    ::RenderFrameLightSubmission* Classify(::LightComponent* light);
};
