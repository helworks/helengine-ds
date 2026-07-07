#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RenderFrameDrawableSubmission;
class IDrawable3D;
class RuntimeMaterial;
class RuntimeSubmesh;
class RuntimeModel;

#include "runtime/array.hpp"
#include "runtime/array.hpp"

class RenderFrameDrawableClassifier
{
public:
    virtual ~RenderFrameDrawableClassifier() = default;

    Array<::RenderFrameDrawableSubmission*>* Classify(::IDrawable3D* drawable);
private:
    static bool IsTransparent(::RuntimeMaterial* material);

    static ::RuntimeMaterial* ResolveMaterial(::IDrawable3D* drawable, int32_t submeshIndex);

    static Array<::RuntimeSubmesh*>* ResolveSubmeshes(::RuntimeModel* model);
};
