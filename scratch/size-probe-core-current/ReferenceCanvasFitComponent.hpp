#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IAnchorBoundsProvider;
class IUpdateable;
class AnchorSpace;
class float2;
class ReferenceCanvasFitSnapshot;
class Entity;
class int2;

#include "UpdateComponent.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_event.hpp"
#include "float2.hpp"
#include "runtime/native_list.hpp"
#include "int2.hpp"

class ReferenceCanvasFitComponent : public ::UpdateComponent, public ::IAnchorBoundsProvider
{
public:
    virtual ~ReferenceCanvasFitComponent() = default;

    ::Event AnchorBoundsChanged;

    int32_t get_ReferenceWidth();

    void set_ReferenceWidth(int32_t value);

    int32_t get_ReferenceHeight();

    void set_ReferenceHeight(int32_t value);

    ::AnchorSpace* get_AnchorSpace();

    ::float2 get_ViewportAnchorOrigin();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    void ParentEnabledChange(bool newEnabled);

    ReferenceCanvasFitComponent();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    int32_t ReferenceWidthValue;

    int32_t ReferenceHeightValue;

    List<::ReferenceCanvasFitSnapshot*>* SnapshotsValue;

    bool IsSubscribedToWindowResizeValue;

    bool PendingApplyValue;

    int32_t SnapshotEntityCountValue;

    ::AnchorSpace* CurrentAnchorSpaceValue;

    ::float2 CurrentCanvasOriginValue;

    void ApplyCurrentScale();

    void AttachToWindowResize();

    void CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity);

    int32_t CountEntitiesRecursive(::Entity* entity);

    void DetachFromWindowResize();

    bool DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::int2 resolvedAnchorSpaceSize, ::float2 resolvedAnchorSpaceOrigin);

    bool DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin);

    void HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight);

    bool LiveWindowMatchesReferenceAspect(double liveWidth, double liveHeight);

    void RaiseAnchorBoundsChanged();

    void RebuildSnapshots();

    void ReleaseSnapshotItems();

    void ReleaseSnapshots();

    ::int2 ResolveCurrentAnchorSpaceSize();

    ::float2 ResolveCurrentCanvasOrigin(::int2 anchorSpaceSize);
};
