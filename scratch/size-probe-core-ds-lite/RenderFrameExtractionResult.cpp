#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameExtractionResult.hpp"
#include "runtime/native_list.hpp"
#include "RenderFrame.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "runtime/native_exceptions.hpp"
#include "RenderFrameExtractionResult.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

List<::RenderFrame*>* RenderFrameExtractionResult::get_Frames()
{
return this->Frames;
}

::RendererBackendCapabilityProfile* RenderFrameExtractionResult::get_BackendCapabilities()
{
return this->BackendCapabilities;
}

RenderFrameExtractionResult::RenderFrameExtractionResult(List<::RenderFrame*>* frames, ::RendererBackendCapabilityProfile* backendCapabilities) : Frames(), BackendCapabilities()
{
this->Frames = (frames != nullptr ? frames : throw new ArgumentNullException("frames"));
this->BackendCapabilities = (backendCapabilities != nullptr ? backendCapabilities : throw new ArgumentNullException("backendCapabilities"));
}

