#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class IAnchorBoundsProvider;
class float4;
class AnchorSpace;
class int2;
class Entity;

#include "Component.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "float4.hpp"
#include "runtime/native_event.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "int2.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_nullable.hpp"

class LayoutComponent : public ::Component, public ::IAnchorBoundsProvider
{
public:
    virtual ~LayoutComponent() = default;

    inline static const uint8_t LeftAnchorFlag = 1;

    inline static const uint8_t RightAnchorFlag = 2;

    inline static const uint8_t TopAnchorFlag = 4;

    inline static const uint8_t BottomAnchorFlag = 8;

    inline static const uint8_t InheritedLayoutSpace = 0;

    inline static const uint8_t ParentLayoutRectSpace = 1;

    inline static const uint8_t ReferenceCanvasLayoutSpace = 2;

    inline static const uint8_t CameraViewportLayoutSpace = 3;

    uint8_t AnchorFlags;

    uint8_t get_AnchorFlags();
    void set_AnchorFlags(uint8_t value);

    ::float4 AnchorDistances;

    ::float4 get_AnchorDistances();
    void set_AnchorDistances(::float4 value);

    ::Event AnchorBoundsChanged;

    uint8_t get_LayoutSpace();

    void set_LayoutSpace(uint8_t value);

    bool get_IsAnchored();

    ::AnchorSpace* get_AnchorSpace();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void DisableAnchoring();

    void Dispose();

    void EnableAnchoring(bool left, bool right, bool top, bool bottom);

    std::string GetAnchorInfo();

    LayoutComponent();

    void ParentEnabledChange(bool newEnabled);

    void RefreshAnchoring();

    void SetAnchorDistances(Nullable<float> left, Nullable<float> right, Nullable<float> top, Nullable<float> bottom);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t LayoutSpaceValue;

    ::IAnchorBoundsProvider* anchorBoundsProvider;

    bool IsSubscribedToWindowResize;

    ::AnchorSpace* FallbackAnchorSpaceValue;

    ::AnchorSpace* ParentAnchorSpaceValue;

    ::AnchorSpace* ChildAnchorSpaceValue;

    ::int2 LastChildAnchorSizeValue;

    void ApplyResolvedSize(::int2 resolvedSize);

    void AttachToWindowResize();

    void DetachFromBoundsProvider();

    void DetachFromWindowResize();

    int32_t GetAnchorArea(::int2 size);

    ::int2 GetAnchorSize();

    ::AnchorSpace* GetAnchorSpace();

    void HandleAnchorBoundsChanged();

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    void PublishOwnAnchorBoundsIfNeeded();

    void RebaseSiblingAnimationPlayers();

    void RefreshSubscriptions();

    ::IAnchorBoundsProvider* ResolveAncestorReferenceCanvasProvider();

    ::IAnchorBoundsProvider* ResolveAncestorViewportProvider();

    ::IAnchorBoundsProvider* ResolveAnchorBoundsProvider();

    ::int2 ResolveImmediateParentAnchorSize();

    ::IAnchorBoundsProvider* ResolveImmediateParentLayoutProvider();

    ::IAnchorBoundsProvider* ResolveInheritedBoundsProvider();
};
