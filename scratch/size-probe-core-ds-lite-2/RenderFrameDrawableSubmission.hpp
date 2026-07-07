#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable3D;
class RuntimeMaterial;
class RenderFrameBatchingMetadata;

class RenderFrameDrawableSubmission
{
public:
    virtual ~RenderFrameDrawableSubmission() = default;

    ::IDrawable3D* Drawable;

    ::IDrawable3D* get_Drawable();

    int32_t SubmeshIndex;

    int32_t get_SubmeshIndex();

    ::RuntimeMaterial* Material;

    ::RuntimeMaterial* get_Material();

    bool IsTransparent;

    bool get_IsTransparent();

    ::RenderFrameBatchingMetadata* BatchingMetadata;

    ::RenderFrameBatchingMetadata* get_BatchingMetadata();

    RenderFrameDrawableSubmission(::IDrawable3D* drawable);

    RenderFrameDrawableSubmission(::IDrawable3D* drawable, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata);

    RenderFrameDrawableSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata);
};
