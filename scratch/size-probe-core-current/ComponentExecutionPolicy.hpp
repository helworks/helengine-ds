#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class Entity;

class ComponentExecutionPolicy
{
public:
    virtual ~ComponentExecutionPolicy() = default;

    static bool ShouldRunComponentLifecycle(::Component* component, ::Entity* entity);
private:
    static bool HasEditorUpdateExecutionSuppressionMarker(::Entity* entity);
};
