#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class RuntimeData;
class MaterialRenderState;

#include "RuntimeData.hpp"
#include "runtime/native_disposable.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/native_list.hpp"

class RuntimeMaterial : public ::RuntimeData, public ::IDisposable
{
public:
    virtual ~RuntimeMaterial() = default;

    ::MaterialRenderState* get_RenderState();

    void set_RenderState(::MaterialRenderState* value);

    ::RuntimeMaterialLightingModel LightingModel;

    ::RuntimeMaterialLightingModel get_LightingModel();
    void set_LightingModel(::RuntimeMaterialLightingModel value);

    bool SupportsNormalMapping;

    bool get_SupportsNormalMapping();
    void set_SupportsNormalMapping(bool value);

    bool SupportsEmissive;

    bool get_SupportsEmissive();
    void set_SupportsEmissive(bool value);

    bool CastsShadows;

    bool get_CastsShadows();
    void set_CastsShadows(bool value);

    bool ReceivesShadows;

    bool get_ReceivesShadows();
    void set_ReceivesShadows(bool value);

    ::RuntimeMaterial* get_ParentMaterial();

    virtual void Dispose();

    ::RuntimeMaterial* ResolveRootMaterial();

    RuntimeMaterial();

    void SetParentMaterial(::RuntimeMaterial* parentMaterial);

    void SetRenderState(::MaterialRenderState* renderState);

    const std::string& get_Id();

    void set_Id(std::string value);
protected:
    void SynchronizeChildMaterials();

    virtual void SynchronizeWithParentMaterial();
private:
    List<::RuntimeMaterial*>* ChildMaterialsValue;

    ::RuntimeMaterial* ParentMaterialValue;

    ::MaterialRenderState* RenderStateValue;

    void RegisterChildMaterial(::RuntimeMaterial* childMaterial);

    void UnregisterChildMaterial(::RuntimeMaterial* childMaterial);

    void ValidateParentMaterial(::RuntimeMaterial* parentMaterial);
};
