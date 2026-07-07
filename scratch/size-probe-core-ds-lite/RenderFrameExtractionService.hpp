#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameExtractionResult;
class CameraComponent;
class IDrawable3D;
class LightComponent;
class RendererBackendCapabilityProfile;

#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class RenderFrameExtractionService
{
public:
    virtual ~RenderFrameExtractionService() = default;

    ::RenderFrameExtractionResult* Extract(List<::CameraComponent*>* cameras, List<::IDrawable3D*>* drawables, List<::LightComponent*>* lights, ::RendererBackendCapabilityProfile* backendCapabilities);
};
