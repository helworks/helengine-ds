#ifdef DrawText
#undef DrawText
#endif
#include "ModelSubmeshResolver.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "ModelAssetIndexData.hpp"
#include "ModelSubmeshAsset.hpp"
#include "ModelSubmeshResolver.hpp"
#include "RuntimeSubmesh.hpp"
#include "ModelAsset.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"

Array<::RuntimeSubmesh*>* ModelSubmeshResolver::BuildRuntimeSubmeshes(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
Array<::ModelSubmeshAsset*> *authoredSubmeshes = ModelSubmeshResolver::ResolveAssetSubmeshes(asset);
Array<::RuntimeSubmesh*> *runtimeSubmeshes = new Array<RuntimeSubmesh*>(authoredSubmeshes->get_Length());
for (int32_t submeshIndex = 0; submeshIndex < authoredSubmeshes->get_Length(); submeshIndex++) {
::ModelSubmeshAsset *authoredSubmesh = (*authoredSubmeshes)[submeshIndex];
(*runtimeSubmeshes)[submeshIndex] = ([&]() {
auto __object_000000FB = new ::RuntimeSubmesh();
__object_000000FB->set_MaterialSlotName(authoredSubmesh->MaterialSlotName);
__object_000000FB->set_IndexStart(authoredSubmesh->IndexStart);
__object_000000FB->set_IndexCount(authoredSubmesh->IndexCount);
return __object_000000FB;
})();
}
return runtimeSubmeshes;}

Array<::ModelSubmeshAsset*>* ModelSubmeshResolver::ResolveAssetSubmeshes(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
const int32_t elementCount = ModelSubmeshResolver::ResolveElementCount(asset);
    if (asset->Submeshes != nullptr && asset->Submeshes->get_Length() > 0)
    {
ModelSubmeshResolver::ValidateSubmeshes(asset->Submeshes, static_cast<int32_t>(elementCount));
return asset->Submeshes;    }
    if (elementCount == 0)
    {
return Array<ModelSubmeshAsset*>::Empty();    }
return new Array<ModelSubmeshAsset*>({ ([&]() {
auto __object_000000FC = new ::ModelSubmeshAsset();
__object_000000FC->set_MaterialSlotName(String::Empty);
__object_000000FC->set_IndexStart(0);
__object_000000FC->set_IndexCount(elementCount);
return __object_000000FC;
})() });}

int32_t ModelSubmeshResolver::ResolveElementCount(::ModelAsset* asset)
{
    if (asset == nullptr)
    {
throw new ArgumentNullException("asset");
    }
::ModelAssetIndexData *indexData = ModelAssetIndexData::Resolve(asset);
    if (indexData->IndexCount > 0)
    {
return indexData->IndexCount;    }
return asset->Positions == nullptr ? 0 : asset->Positions->get_Length();}

void ModelSubmeshResolver::ValidateSubmeshes(Array<::ModelSubmeshAsset*>* submeshes, int32_t elementCount)
{
    if (submeshes == nullptr)
    {
throw new ArgumentNullException("submeshes");
    }
    if (elementCount < 0)
    {
throw ([&]() {
auto __ctor_arg_000000FD = "elementCount";
auto __ctor_arg_000000FE = "Element count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000000FD, __ctor_arg_000000FE);
})();
    }
for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
::ModelSubmeshAsset *submesh = (*submeshes)[submeshIndex];
    if (submesh == nullptr)
    {
throw new InvalidOperationException("Model submesh collections cannot contain null entries.");
    }
    if (submesh->IndexStart < 0)
    {
throw new InvalidOperationException("Model submesh index starts must be zero or greater.");
    }
    if (submesh->IndexCount <= 0)
    {
throw new InvalidOperationException("Model submesh index counts must be greater than zero.");
    }
    if (submesh->IndexStart + submesh->IndexCount > elementCount)
    {
throw new InvalidOperationException("Model submesh ranges cannot exceed the resolved model element count.");
    }
}
}

