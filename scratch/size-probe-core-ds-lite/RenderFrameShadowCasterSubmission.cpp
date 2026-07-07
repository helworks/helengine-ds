#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameShadowCasterSubmission.hpp"
#include "RuntimeMaterial.hpp"
#include "IDrawable3D.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RenderFrameShadowCasterSubmission.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

::IDrawable3D* RenderFrameShadowCasterSubmission::get_Drawable()
{
return this->Drawable;
}

int32_t RenderFrameShadowCasterSubmission::get_SubmeshIndex()
{
return this->SubmeshIndex;
}

::RuntimeMaterial* RenderFrameShadowCasterSubmission::get_Material()
{
return this->Material;
}

RenderFrameShadowCasterSubmission::RenderFrameShadowCasterSubmission(::IDrawable3D* drawable) : Drawable(), SubmeshIndex(0), Material()
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
Array<::RuntimeMaterial*> *materials = drawable->get_Materials();
this->Material = materials != nullptr && materials->get_Length() > 0 ? (*materials)[0] : nullptr;
}

RenderFrameShadowCasterSubmission::RenderFrameShadowCasterSubmission(::IDrawable3D* drawable, int32_t submeshIndex, ::RuntimeMaterial* material) : Drawable(), SubmeshIndex(0), Material()
{
this->Drawable = (drawable != nullptr ? drawable : throw new ArgumentNullException("drawable"));
    if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000123 = "submeshIndex";
auto __ctor_arg_00000124 = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000123, __ctor_arg_00000124);
})();
    }
this->SubmeshIndex = submeshIndex;
this->Material = material;
}

