#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrame;
class RendererBackendCapabilityProfile;

#include "runtime/native_list.hpp"

class RenderFrameExtractionResult
{
public:
    virtual ~RenderFrameExtractionResult() = default;

    List<::RenderFrame*>* Frames;

    List<::RenderFrame*>* get_Frames();

    ::RendererBackendCapabilityProfile* BackendCapabilities;

    ::RendererBackendCapabilityProfile* get_BackendCapabilities();

    RenderFrameExtractionResult(List<::RenderFrame*>* frames, ::RendererBackendCapabilityProfile* backendCapabilities);
};
