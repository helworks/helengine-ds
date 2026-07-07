#ifdef DrawText
#undef DrawText
#endif
#include "RenderFrameDrawableClassifier.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "IDrawable3D.hpp"
#include "RuntimeMaterial.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeSubmesh.hpp"
#include "RenderFrameDrawableClassifier.hpp"
#include "RenderFrameDrawableSubmission.hpp"
#include "RenderFrameBatchingMetadata.hpp"
#include "MaterialBlendMode.hpp"
#include "RuntimeModel.hpp"
#include "Entity.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/native_list.hpp"
#include "MaterialCullMode.hpp"
#include "runtime/native_string.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<::RenderFrameDrawableSubmission*>* RenderFrameDrawableClassifier::Classify(::IDrawable3D* drawable)
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
Array<::RuntimeSubmesh*> *submeshes = RenderFrameDrawableClassifier::ResolveSubmeshes(drawable->get_Model());
Array<::RenderFrameDrawableSubmission*> *submissions = new Array<RenderFrameDrawableSubmission*>(submeshes->get_Length());
for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
::RuntimeMaterial *material = RenderFrameDrawableClassifier::ResolveMaterial(drawable, static_cast<int32_t>(submeshIndex));
(*submissions)[submeshIndex] = ([&]() {
auto __ctor_arg_00000111 = drawable;
auto __ctor_arg_00000112 = static_cast<int32_t>(submeshIndex);
auto __ctor_arg_00000113 = material;
auto __ctor_arg_00000114 = RenderFrameDrawableClassifier::IsTransparent(material);
auto __ctor_arg_00000115 = new ::RenderFrameBatchingMetadata(false, false, false);
return new ::RenderFrameDrawableSubmission(__ctor_arg_00000111, __ctor_arg_00000112, __ctor_arg_00000113, __ctor_arg_00000114, __ctor_arg_00000115);
})();
}
return submissions;}

bool RenderFrameDrawableClassifier::IsTransparent(::RuntimeMaterial* material)
{
    if (material == nullptr)
    {
return false;    }
::MaterialRenderState *renderState = material->get_RenderState();
return renderState != nullptr && renderState->BlendMode == MaterialBlendMode::AlphaBlend;}

::RuntimeMaterial* RenderFrameDrawableClassifier::ResolveMaterial(::IDrawable3D* drawable, int32_t submeshIndex)
{
    if (drawable == nullptr)
    {
throw new ArgumentNullException("drawable");
    }
else {
    if (submeshIndex < 0)
    {
throw ([&]() {
auto __ctor_arg_00000116 = "submeshIndex";
auto __ctor_arg_00000117 = "Submesh index must be non-negative.";
return new ArgumentOutOfRangeException(__ctor_arg_00000116, __ctor_arg_00000117);
})();
    }
}
Array<::RuntimeMaterial*> *materials = drawable->get_Materials();
    if (materials == nullptr || materials->get_Length() == 0)
    {
return nullptr;    }
    if (submeshIndex < materials->get_Length())
    {
return (*materials)[submeshIndex];    }
return (*materials)[0];}

Array<::RuntimeSubmesh*>* RenderFrameDrawableClassifier::ResolveSubmeshes(::RuntimeModel* model)
{
    if (model == nullptr || model->get_Submeshes() == nullptr || model->get_Submeshes()->get_Length() == 0)
    {
return new Array<RuntimeSubmesh*>({ ([&]() {
auto __object_00000118 = new ::RuntimeSubmesh();
__object_00000118->set_MaterialSlotName(String::Empty);
__object_00000118->set_IndexStart(0);
__object_00000118->set_IndexCount(0);
return __object_00000118;
})() });    }
return model->get_Submeshes();}

