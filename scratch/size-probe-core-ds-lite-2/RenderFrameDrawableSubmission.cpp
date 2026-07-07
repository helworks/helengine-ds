#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "RuntimeMaterial.hpp"
#include "IDrawable3D.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::IDrawable3D* RenderFrameDrawableSubmission::get_Drawable()
{
return this->Drawable;
}

int32_t RenderFrameDrawableSubmission::get_SubmeshIndex()
{
return this->SubmeshIndex;
}

::RuntimeMaterial* RenderFrameDrawableSubmission::get_Material()
{
return this->Material;
}

bool RenderFrameDrawableSubmission::get_IsTransparent()
{
return this->IsTransparent;
}

::RenderFrameBatchingMetadata* RenderFrameDrawableSubmission::get_BatchingMetadata()
{
return this->BatchingMetadata;
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable) : Drawable(), SubmeshIndex(0), Material(), IsTransparent(), BatchingMetadata()
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
this->Drawable = drawable;
this->BatchingMetadata = new ::RenderFrameBatchingMetadata(false, false, false);
Array<::RuntimeMaterial*> *materials = drawable->get_Materials();
this->Material = materials != nullptr && materials->get_Length() > 0 ? (*materials)[0] : nullptr;
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata) : Drawable(), SubmeshIndex(0), Material(), IsTransparent(), BatchingMetadata()
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
this->BatchingMetadata = (batchingMetadata != nullptr ? batchingMetadata : throw new ArgumentNullException("batchingMetadata"));
Array<::RuntimeMaterial*> *materials = drawable->get_Materials();
this->Material = materials != nullptr && materials->get_Length() > 0 ? (*materials)[0] : nullptr;
this->IsTransparent = isTransparent;
}

RenderFrameDrawableSubmission::RenderFrameDrawableSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material, bool isTransparent, ::RenderFrameBatchingMetadata* batchingMetadata) : Drawable(), SubmeshIndex(0), Material(), IsTransparent(), BatchingMetadata()
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
    if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000119 = "submeshIndex";
auto __ctor_arg_0000011A = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000119, __ctor_arg_0000011A);
})();
    }
this->BatchingMetadata = (batchingMetadata != nullptr ? batchingMetadata : throw new ArgumentNullException("batchingMetadata"));
this->SubmeshIndex = submeshIndex;
this->Material = material;
this->IsTransparent = isTransparent;
}

