#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeModel.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
#include "RuntimeModel.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"

Array<::RuntimeSubmesh*>* RuntimeModel::get_Submeshes()
{
return this->SubmeshesValue;
}

void RuntimeModel::set_Submeshes(Array<::RuntimeSubmesh*>* value)
{
this->SubmeshesValue = value;
}

::float3 RuntimeModel::get_BoundsMin()
{
return this->BoundsMin;
}

void RuntimeModel::set_BoundsMin(::float3 value)
{
this->BoundsMin = value;
}

::float3 RuntimeModel::get_BoundsMax()
{
return this->BoundsMax;
}

void RuntimeModel::set_BoundsMax(::float3 value)
{
this->BoundsMax = value;
}

void RuntimeModel::Dispose()
{
    if (!(this->SubmeshesValue == Array<RuntimeSubmesh*>::Empty()))
    {
NativeOwnership::DeleteItemsAndRelease__ref0<RuntimeSubmesh*>(this->SubmeshesValue);
    }
this->SubmeshesValue = nullptr;
}

void RuntimeModel::SetBounds(::float3 boundsMin, ::float3 boundsMax)
{
this->set_BoundsMin(boundsMin);
this->set_BoundsMax(boundsMax);
}

void RuntimeModel::SetSubmeshes(Array<::RuntimeSubmesh*>* submeshes)
{
    if (submeshes == nullptr)
    {
throw new ArgumentNullException("submeshes");
    }
Array<::RuntimeSubmesh*> *copiedSubmeshes = new Array<RuntimeSubmesh*>(submeshes->get_Length());
for (int32_t submeshIndex = 0; submeshIndex < submeshes->get_Length(); submeshIndex++) {
::RuntimeSubmesh *submesh = (*submeshes)[submeshIndex];
    if (submesh == nullptr)
    {
throw new InvalidOperationException("Runtime model submesh collections cannot contain null entries.");
    }
(*copiedSubmeshes)[submeshIndex] = submesh;
}
this->set_Submeshes(copiedSubmeshes);
}

RuntimeModel::RuntimeModel() : BoundsMin(), BoundsMax(), SubmeshesValue()
{
this->SubmeshesValue = Array<RuntimeSubmesh*>::Empty();
}

