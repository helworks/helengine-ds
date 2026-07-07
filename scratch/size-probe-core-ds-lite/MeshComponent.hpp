#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IDrawable3D;
class RuntimeModel;
class RuntimeMaterial;
class Entity;

#include "Component.hpp"
#include "IDrawable3D.hpp"
#include "runtime/native_disposable.hpp"
#include "runtime/array.hpp"

class MeshComponent : public ::Component, public ::IDrawable3D
{
public:
    virtual ~MeshComponent() = default;

    ::RuntimeModel* Model;

    ::RuntimeModel* get_Model();
    void set_Model(::RuntimeModel* value);

    Array<::RuntimeMaterial*>* get_Materials();

    void set_Materials(Array<::RuntimeMaterial*>* value);

    uint8_t get_RenderOrder3D();

    void set_RenderOrder3D(uint8_t value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    MeshComponent();

    void ParentEnabledChange(bool newEnabled);

    void SetMaterials(Array<::RuntimeMaterial*>* runtimeMaterials);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t renderOrder3D;

    Array<::RuntimeMaterial*>* MaterialsBySlot;
};
