#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;

#include "Component.hpp"
#include "runtime/native_disposable.hpp"

class LineRendererComponent : public ::Component
{
public:
    virtual ~LineRendererComponent() = default;

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
};
