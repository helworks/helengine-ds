#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeMaterial.hpp"
#include "RuntimeMaterial.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_exceptions.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/array.hpp"
#include "MaterialBlendMode.hpp"
#include "MaterialCullMode.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"

::MaterialRenderState* RuntimeMaterial::get_RenderState()
{
return this->RenderStateValue;
}

void RuntimeMaterial::set_RenderState(::MaterialRenderState* value)
{
this->RenderStateValue = value;
}

::RuntimeMaterialLightingModel RuntimeMaterial::get_LightingModel()
{
return this->LightingModel;
}

void RuntimeMaterial::set_LightingModel(::RuntimeMaterialLightingModel value)
{
this->LightingModel = value;
}

bool RuntimeMaterial::get_SupportsNormalMapping()
{
return this->SupportsNormalMapping;
}

void RuntimeMaterial::set_SupportsNormalMapping(bool value)
{
this->SupportsNormalMapping = value;
}

bool RuntimeMaterial::get_SupportsEmissive()
{
return this->SupportsEmissive;
}

void RuntimeMaterial::set_SupportsEmissive(bool value)
{
this->SupportsEmissive = value;
}

bool RuntimeMaterial::get_CastsShadows()
{
return this->CastsShadows;
}

void RuntimeMaterial::set_CastsShadows(bool value)
{
this->CastsShadows = value;
}

bool RuntimeMaterial::get_ReceivesShadows()
{
return this->ReceivesShadows;
}

void RuntimeMaterial::set_ReceivesShadows(bool value)
{
this->ReceivesShadows = value;
}

::RuntimeMaterial* RuntimeMaterial::get_ParentMaterial()
{
return this->ParentMaterialValue;
}

void RuntimeMaterial::Dispose()
{
::RuntimeMaterial *parentMaterial = this->ParentMaterialValue;
    if (parentMaterial != nullptr)
    {
parentMaterial->UnregisterChildMaterial(this);
    }
this->ParentMaterialValue = nullptr;
delete this->RenderStateValue;
this->RenderStateValue = nullptr;
this->ChildMaterialsValue->Clear();
delete this->ChildMaterialsValue;
}

::RuntimeMaterial* RuntimeMaterial::ResolveRootMaterial()
{
::RuntimeMaterial *material = this;
while (material->ParentMaterialValue != nullptr) {
material = material->ParentMaterialValue;
}
return material;}

RuntimeMaterial::RuntimeMaterial() : LightingModel(), SupportsNormalMapping(), SupportsEmissive(), CastsShadows(), ReceivesShadows(), ChildMaterialsValue(), ParentMaterialValue(), RenderStateValue()
{
this->RenderStateValue = new ::MaterialRenderState();
this->ChildMaterialsValue = new List<::RuntimeMaterial*>();
this->set_LightingModel(RuntimeMaterialLightingModel::Unlit);
this->set_SupportsNormalMapping(false);
this->set_SupportsEmissive(false);
this->set_CastsShadows(true);
this->set_ReceivesShadows(true);
}

void RuntimeMaterial::SetParentMaterial(::RuntimeMaterial* parentMaterial)
{
    if (parentMaterial == nullptr)
    {
throw new ArgumentNullException("parentMaterial");
    }
    if ((this->ParentMaterialValue == parentMaterial))
    {
return;    }
this->ValidateParentMaterial(parentMaterial);
::RuntimeMaterial *previousParentMaterial = this->ParentMaterialValue;
    if (previousParentMaterial != nullptr)
    {
previousParentMaterial->UnregisterChildMaterial(this);
    }
this->ParentMaterialValue = parentMaterial;
parentMaterial->RegisterChildMaterial(this);
this->SynchronizeWithParentMaterial();
}

void RuntimeMaterial::SetRenderState(::MaterialRenderState* renderState)
{
    if (renderState == nullptr)
    {
throw new ArgumentNullException("renderState");
    }
else {
    if (this->ParentMaterialValue != nullptr)
    {
throw new InvalidOperationException("Parented runtime materials inherit their render state from the parent material.");
    }
}
::MaterialRenderState *previousRenderState = this->get_RenderState();
this->set_RenderState(renderState->Clone());
delete previousRenderState;
this->SynchronizeChildMaterials();
}

const std::string& RuntimeMaterial::get_Id()
{
return RuntimeData::get_Id();
}

void RuntimeMaterial::set_Id(std::string value)
{
RuntimeData::set_Id(value);
}

void RuntimeMaterial::SynchronizeChildMaterials()
{
for (int32_t childIndex = 0; childIndex < this->ChildMaterialsValue->get_Count(); childIndex++) {
::RuntimeMaterial *childMaterial = (*this->ChildMaterialsValue).get_Item(static_cast<int32_t>(childIndex));
    if (childMaterial == nullptr)
    {
continue;
    }
childMaterial->SynchronizeWithParentMaterial();
}
}

void RuntimeMaterial::SynchronizeWithParentMaterial()
{
    if (this->ParentMaterialValue == nullptr)
    {
return;    }
::MaterialRenderState *previousRenderState = this->get_RenderState();
this->set_RenderState(this->ParentMaterialValue->get_RenderState()->Clone());
delete previousRenderState;
this->SynchronizeChildMaterials();
}

void RuntimeMaterial::RegisterChildMaterial(::RuntimeMaterial* childMaterial)
{
    if (childMaterial == nullptr)
    {
throw new ArgumentNullException("childMaterial");
    }
else {
    if (this->ChildMaterialsValue->Contains(childMaterial))
    {
throw new InvalidOperationException("Child materials cannot be registered to the same parent more than once.");
    }
}
this->ChildMaterialsValue->Add(childMaterial);
}

void RuntimeMaterial::UnregisterChildMaterial(::RuntimeMaterial* childMaterial)
{
    if (childMaterial == nullptr)
    {
throw new ArgumentNullException("childMaterial");
    }
this->ChildMaterialsValue->Remove(childMaterial);
}

void RuntimeMaterial::ValidateParentMaterial(::RuntimeMaterial* parentMaterial)
{
::RuntimeMaterial *currentMaterial = parentMaterial;
while (currentMaterial != nullptr) {
    if ((currentMaterial == this))
    {
throw new InvalidOperationException("Runtime materials cannot inherit from themselves or from one of their children.");
    }
currentMaterial = currentMaterial->ParentMaterialValue;
}
}

