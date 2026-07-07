#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IClipRegion2D;
class IUpdateable;
class int2;
class Entity;
class float4;

#include "UpdateComponent.hpp"
#include "IClipRegion2D.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "float4.hpp"

class ScrollComponent : public ::UpdateComponent, public ::IClipRegion2D
{
public:
    virtual ~ScrollComponent() = default;

    ScrollComponent();

    ::Event ScrollOffsetChanged;

    ::int2 get_Size();

    void set_Size(::int2 value);

    int32_t get_ItemCount();

    void set_ItemCount(int32_t value);

    int32_t get_VisibleItemCount();

    void set_VisibleItemCount(int32_t value);

    bool get_UsesAutomaticVisibleItemCount();

    int32_t get_ItemExtent();

    void set_ItemExtent(int32_t value);

    int32_t get_MaximumScrollOffset();

    int32_t ScrollOffset;

    int32_t get_ScrollOffset();
    void set_ScrollOffset(int32_t value);

    ::Entity* get_ContentRoot();

    void set_ContentRoot(::Entity* value);

    ::Entity* get_ClipOriginEntity();

    void set_ClipOriginEntity(::Entity* value);

    int32_t get_ScrollStepCount();

    void set_ScrollStepCount(int32_t value);

    int32_t get_WheelNotchSize();

    void set_WheelNotchSize(int32_t value);

    bool get_RequiresPointerInside();

    void set_RequiresPointerInside(bool value);

    void ClampScrollOffset();

    bool ContainsScreenPoint(int32_t x, int32_t y);

    ::float4 GetClipRect();

    void ResetScrollOffset();

    bool ScrollTo(int32_t scrollOffset);

    bool TryApplyWheelInput();

    void Update();

    ::Entity* get_Parent();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    inline static const int32_t StandardWheelNotch = 120;

    ::int2 SizeValue;

    int32_t ItemCountValue;

    int32_t ItemExtentValue;

    int32_t VisibleItemCountValue;

    int32_t ScrollStepCountValue;

    int32_t WheelNotchSizeValue;

    bool RequiresPointerInsideValue;

    ::Entity* ContentRootValue;

    ::Entity* ClipOriginEntityValue;

    void ApplyContentRootOffset();

    int32_t ClampOffset(int32_t scrollOffset);

    int32_t GetVisibleItemCount();

    ::int2 ResolveViewportSize();

    bool SetScrollOffset(int32_t scrollOffset, bool raiseEvent);
};
