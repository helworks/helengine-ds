#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IClipRegion2D;
class IAnchorSizeProvider;
class int2;
class float4;

#include "Component.hpp"
#include "IClipRegion2D.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "int2.hpp"
#include "float4.hpp"

class ClipRectComponent : public ::Component, public ::IClipRegion2D, public ::IAnchorSizeProvider
{
public:
    virtual ~ClipRectComponent() = default;

    ClipRectComponent();

    ::int2 get_Size();

    void set_Size(::int2 value);

    ::int2 get_AnchorSize();

    ::float4 GetClipRect();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::int2 SizeValue;
};
