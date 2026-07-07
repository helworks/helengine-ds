#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IAnchorBoundsProvider;
class ICameraBoundViewportOwner;
class IUpdateable;
class int2;
class CameraComponent;
class AnchorSpace;
class float4;
class ViewportLayoutSnapshot;
class float2;
class Entity;

#include "UpdateComponent.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "ICameraBoundViewportOwner.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "float4.hpp"
#include "runtime/native_list.hpp"
#include "float2.hpp"

class ViewportComponent : public ::UpdateComponent, public ::IAnchorBoundsProvider, public ::ICameraBoundViewportOwner
{
public:
    virtual ~ViewportComponent() = default;

    inline static const uint8_t ScreenBindingMode = 0;

    inline static const uint8_t AncestorCameraBindingMode = 1;

    inline static const uint8_t FixedBindingMode = 2;

    inline static const uint8_t ExplicitCameraBindingMode = 3;

    inline static const uint8_t NoScalingMode = 0;

    inline static const uint8_t ReferenceCanvasScalingMode = 1;

    ::Event AnchorBoundsChanged;

    uint8_t get_BindingMode();

    void set_BindingMode(uint8_t value);

    ::int2 get_FixedSize();

    void set_FixedSize(::int2 value);

    uint8_t get_ScalingMode();

    void set_ScalingMode(uint8_t value);

    int32_t get_ReferenceWidth();

    void set_ReferenceWidth(int32_t value);

    int32_t get_ReferenceHeight();

    void set_ReferenceHeight(int32_t value);

    ::CameraComponent* get_BoundCameraComponent();

    void set_BoundCameraComponent(::CameraComponent* value);

    ::AnchorSpace* get_AnchorSpace();

    ::AnchorSpace* get_ViewportAnchorSpace();

    ::float4 get_ResolvedViewportBounds();

    ::int2 get_ResolvedViewportSize();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    ::CameraComponent* GetBoundCameraComponent();

    void ParentEnabledChange(bool newEnabled);

    void Update();

    ViewportComponent();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t BindingModeValue;

    ::int2 FixedSizeValue;

    ::CameraComponent* ActiveCameraComponentValue;

    ::CameraComponent* ExplicitBoundCameraComponentValue;

    uint8_t ScalingModeValue;

    int32_t ReferenceWidthValue;

    int32_t ReferenceHeightValue;

    List<::ViewportLayoutSnapshot*>* LayoutSnapshotsValue;

    bool IsSubscribedToWindowResizeValue;

    bool PendingScaleApplyValue;

    int32_t SnapshotEntityCountValue;

    ::AnchorSpace* CurrentAnchorSpaceValue;

    ::AnchorSpace* CurrentViewportAnchorSpaceValue;

    ::float2 CurrentCanvasOriginValue;

    void ApplyCurrentScale();

    void AttachToCamera();

    void AttachToWindowResize();

    void CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity);

    int32_t CountEntitiesRecursive(::Entity* entity);

    void DetachFromCamera();

    void DetachFromWindowResize();

    bool DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::int2 resolvedAnchorSpaceSize, ::float2 resolvedAnchorSpaceOrigin);

    bool DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin);

    void HandleCameraViewportChanged();

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    bool LiveViewportMatchesReferenceAspect(double liveWidth, double liveHeight);

    void RaiseAnchorBoundsChanged();

    void RebuildSnapshots();

    void RefreshSubscriptions();

    void ReleaseLayoutSnapshotItems();

    void ReleaseLayoutSnapshots();

    ::CameraComponent* ResolveAncestorCameraComponent();

    ::int2 ResolveAnchorBounds();

    ::CameraComponent* ResolveBoundCameraComponent();

    ::float4 ResolveCameraViewportBounds(::CameraComponent* cameraComponent);

    ::int2 ResolveCurrentAnchorSpaceSize();

    ::float2 ResolveCurrentCanvasOrigin(::int2 anchorSpaceSize);

    ::float2 ResolveViewportAnchorOrigin();

    ::float4 ResolveViewportBounds();
};
