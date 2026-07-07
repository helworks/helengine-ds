#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IUpdateable;
class Entity;

#include "Component.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_disposable.hpp"

class UpdateComponent : public ::Component, public ::IUpdateable
{
public:
    virtual ~UpdateComponent() = default;

    UpdateComponent();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    void ComponentAdded(::Entity* entity);

    void ParentEnabledChange(bool newEnabled);

    virtual void Update();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t updateOrder;
};
