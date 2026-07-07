#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class TextBoxComponent;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"

class TextBoxUpdateComponent : public ::UpdateComponent
{
public:
    virtual ~TextBoxUpdateComponent() = default;

    TextBoxUpdateComponent(::TextBoxComponent* textBox);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::TextBoxComponent* textBox;
};
