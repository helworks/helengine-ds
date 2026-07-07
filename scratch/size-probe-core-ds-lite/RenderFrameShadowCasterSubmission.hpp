#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IDrawable3D;
class RuntimeMaterial;

class RenderFrameShadowCasterSubmission
{
public:
    virtual ~RenderFrameShadowCasterSubmission() = default;

    ::IDrawable3D* Drawable;

    ::IDrawable3D* get_Drawable();

    int32_t SubmeshIndex;

    int32_t get_SubmeshIndex();

    ::RuntimeMaterial* Material;

    ::RuntimeMaterial* get_Material();

    RenderFrameShadowCasterSubmission(::IDrawable3D* drawable);

    RenderFrameShadowCasterSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material);
};
