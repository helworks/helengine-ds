#ifdef DrawText
#undef DrawText
#endif
#include "ReferenceCanvasFitComponent.hpp"
#include "UpdateComponent.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_list.hpp"
#include "int2.hpp"
#include "float2.hpp"
#include "AnchorSpace.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "system/math.hpp"
#include "SceneCanvasProfile.hpp"
#include "ReferenceCanvasFitSnapshot.hpp"
#include "runtime/native_event.hpp"
#include "Entity.hpp"
#include "runtime/array.hpp"
#include "runtime/native_string.hpp"
#include "float3.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "FontAsset.hpp"
#include "RenderManager2D.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "ModelAsset.hpp"
#include "RenderTarget.hpp"
#include "RendererBackendCapabilityProfile.hpp"
#include "LayoutComponent.hpp"
#include "float4.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "SpriteComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "ReferenceCanvasFitComponent.hpp"
#include "Component.hpp"
#include "system/action.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

int32_t ReferenceCanvasFitComponent::get_ReferenceWidth()
{
return this->ReferenceWidthValue;}

void ReferenceCanvasFitComponent::set_ReferenceWidth(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_000001FA = "value";
auto __ctor_arg_000001FB = "Reference canvas width must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FA, __ctor_arg_000001FB);
})();
    }
this->ReferenceWidthValue = value;
this->ApplyCurrentScale();
}

int32_t ReferenceCanvasFitComponent::get_ReferenceHeight()
{
return this->ReferenceHeightValue;}

void ReferenceCanvasFitComponent::set_ReferenceHeight(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_000001FC = "value";
auto __ctor_arg_000001FD = "Reference canvas height must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FC, __ctor_arg_000001FD);
})();
    }
this->ReferenceHeightValue = value;
this->ApplyCurrentScale();
}

::AnchorSpace* ReferenceCanvasFitComponent::get_AnchorSpace()
{
return this->CurrentAnchorSpaceValue;
}

::float2 ReferenceCanvasFitComponent::get_ViewportAnchorOrigin()
{
return ::float2(-this->CurrentCanvasOriginValue.X, -this->CurrentCanvasOriginValue.Y);
}

void ReferenceCanvasFitComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
this->RebuildSnapshots();
this->AttachToWindowResize();
this->PendingApplyValue = true;
}

void ReferenceCanvasFitComponent::ComponentRemoved(::Entity* entity)
{
UpdateComponent::ComponentRemoved(entity);
this->DetachFromWindowResize();
this->ReleaseSnapshotItems();
this->SnapshotEntityCountValue = 0;
this->PendingApplyValue = false;
}

void ReferenceCanvasFitComponent::Dispose()
{
this->DetachFromWindowResize();
this->ReleaseSnapshots();
delete this->CurrentAnchorSpaceValue;
this->CurrentAnchorSpaceValue = nullptr;
UpdateComponent::Dispose();
}

void ReferenceCanvasFitComponent::ParentEnabledChange(bool newEnabled)
{
UpdateComponent::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
this->AttachToWindowResize();
this->PendingApplyValue = true;
return;    }
this->DetachFromWindowResize();
}

ReferenceCanvasFitComponent::ReferenceCanvasFitComponent() : AnchorBoundsChanged(), ReferenceWidthValue(0), ReferenceHeightValue(0), SnapshotsValue(), IsSubscribedToWindowResizeValue(), PendingApplyValue(), SnapshotEntityCountValue(0), CurrentAnchorSpaceValue(), CurrentCanvasOriginValue()
{
this->ReferenceWidthValue = SceneCanvasProfile::DefaultWidth;
this->ReferenceHeightValue = SceneCanvasProfile::DefaultHeight;
this->SnapshotsValue = new List<::ReferenceCanvasFitSnapshot*>();
this->CurrentAnchorSpaceValue = ([&]() {
auto __ctor_arg_000001FE = ::int2(static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));
auto __ctor_arg_000001FF = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_000001FE, __ctor_arg_000001FF);
})();
this->CurrentCanvasOriginValue = ::float2(0.0f, 0.0f);
}

void ReferenceCanvasFitComponent::Update()
{
UpdateComponent::Update();
    if (this->Parent == nullptr)
    {
return;    }
const int32_t currentEntityCount = this->CountEntitiesRecursive(this->Parent);
    if (currentEntityCount != this->SnapshotEntityCountValue)
    {
this->RebuildSnapshots();
this->PendingApplyValue = true;
    }
    if (this->PendingApplyValue)
    {
this->ApplyCurrentScale();
this->PendingApplyValue = false;
    }
}

uint8_t ReferenceCanvasFitComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void ReferenceCanvasFitComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* ReferenceCanvasFitComponent::get_Parent()
{
return Component::get_Parent();
}

void ReferenceCanvasFitComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ReferenceCanvasFitComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ReferenceCanvasFitComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ReferenceCanvasFitComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void ReferenceCanvasFitComponent::ApplyCurrentScale()
{
    if (this->Parent == nullptr || Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr || this->SnapshotsValue->get_Count() == 0)
    {
return;    }
::int2 resolvedAnchorSpaceSize = this->ResolveCurrentAnchorSpaceSize();
::float2 resolvedAnchorSpaceOrigin = ::float2(0.0f, 0.0f);
::float2 resolvedCanvasOrigin = this->ResolveCurrentCanvasOrigin(resolvedAnchorSpaceSize);
const bool anchorSpaceChanged = this->DidAnchorSpaceChange(this->CurrentAnchorSpaceValue, resolvedAnchorSpaceSize, resolvedAnchorSpaceOrigin) || this->DidCanvasOriginChange(this->CurrentCanvasOriginValue, resolvedCanvasOrigin);
this->CurrentAnchorSpaceValue->Update(resolvedAnchorSpaceSize, resolvedAnchorSpaceOrigin);
this->CurrentCanvasOriginValue = resolvedCanvasOrigin;
for (int32_t snapshotIndex = 0; snapshotIndex < this->SnapshotsValue->get_Count(); snapshotIndex++) {
(*this->SnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex))->Apply(this->CurrentAnchorSpaceValue, resolvedCanvasOrigin, static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));
}
for (int32_t snapshotIndex = 0; snapshotIndex < this->SnapshotsValue->get_Count(); snapshotIndex++) {
(*this->SnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex))->RefreshAnchoring();
}
    if (anchorSpaceChanged)
    {
this->RaiseAnchorBoundsChanged();
    }
}

void ReferenceCanvasFitComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResizeValue)
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
return;    }
Core::Instance->RenderManager3D->WindowResized += Event::Bind(this, static_cast<void (ReferenceCanvasFitComponent::*)(intptr_t, int32_t, int32_t)>(&ReferenceCanvasFitComponent::HandleWindowResized));
this->IsSubscribedToWindowResizeValue = true;
}

void ReferenceCanvasFitComponent::CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity)
{
this->SnapshotsValue->Add(new ::ReferenceCanvasFitSnapshot(entity, isRootEntity));
    if (entity->get_Children() == nullptr)
    {
return;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->get_Count(); childIndex++) {
this->CaptureSnapshotsRecursive((*entity->get_Children()).get_Item(static_cast<int32_t>(childIndex)), false);
}
}

int32_t ReferenceCanvasFitComponent::CountEntitiesRecursive(::Entity* entity)
{
    if (entity == nullptr)
    {
return 0;    }
int32_t count = 1;
    if (entity->get_Children() == nullptr)
    {
return count;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->get_Count(); childIndex++) {
count += this->CountEntitiesRecursive((*entity->get_Children()).get_Item(static_cast<int32_t>(childIndex)));
}
return count;}

void ReferenceCanvasFitComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResizeValue)
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
this->IsSubscribedToWindowResizeValue = false;
return;    }
Core::Instance->RenderManager3D->WindowResized -= Event::Bind(this, static_cast<void (ReferenceCanvasFitComponent::*)(intptr_t, int32_t, int32_t)>(&ReferenceCanvasFitComponent::HandleWindowResized));
this->IsSubscribedToWindowResizeValue = false;
}

bool ReferenceCanvasFitComponent::DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::int2 resolvedAnchorSpaceSize, ::float2 resolvedAnchorSpaceOrigin)
{
    if (currentAnchorSpace == nullptr)
    {
return true;    }
return currentAnchorSpace->Size.X != resolvedAnchorSpaceSize.X || currentAnchorSpace->Size.Y != resolvedAnchorSpaceSize.Y || currentAnchorSpace->Origin.X != resolvedAnchorSpaceOrigin.X || currentAnchorSpace->Origin.Y != resolvedAnchorSpaceOrigin.Y;}

bool ReferenceCanvasFitComponent::DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin)
{
return currentCanvasOrigin.X != resolvedCanvasOrigin.X || currentCanvasOrigin.Y != resolvedCanvasOrigin.Y;}

void ReferenceCanvasFitComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->PendingApplyValue = true;
}

bool ReferenceCanvasFitComponent::LiveWindowMatchesReferenceAspect(double liveWidth, double liveHeight)
{
const double expectedWidth = liveHeight * this->ReferenceWidthValue / this->ReferenceHeightValue;
const double expectedHeight = liveWidth * this->ReferenceHeightValue / this->ReferenceWidthValue;
return Math::Abs(liveWidth - expectedWidth) <= 0.5 || Math::Abs(liveHeight - expectedHeight) <= 0.5;}

void ReferenceCanvasFitComponent::RaiseAnchorBoundsChanged()
{
    if (true)
    {
this->AnchorBoundsChanged.Invoke();
    }
}

void ReferenceCanvasFitComponent::RebuildSnapshots()
{
this->ReleaseSnapshotItems();
    if (this->Parent == nullptr)
    {
this->SnapshotEntityCountValue = 0;
return;    }
this->CaptureSnapshotsRecursive(this->Parent, true);
this->SnapshotEntityCountValue = this->SnapshotsValue->get_Count();
}

void ReferenceCanvasFitComponent::ReleaseSnapshotItems()
{
for (int32_t snapshotIndex = 0; snapshotIndex < this->SnapshotsValue->get_Count(); snapshotIndex++) {
delete (*this->SnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex));
}
this->SnapshotsValue->Clear();
}

void ReferenceCanvasFitComponent::ReleaseSnapshots()
{
this->ReleaseSnapshotItems();
delete this->SnapshotsValue;
}

::int2 ReferenceCanvasFitComponent::ResolveCurrentAnchorSpaceSize()
{
::int2 mainWindowSize = Core::Instance->RenderManager3D->MainWindowSize;
const double liveWidth = mainWindowSize.X > 0 ? mainWindowSize.X : this->ReferenceWidthValue;
const double liveHeight = mainWindowSize.Y > 0 ? mainWindowSize.Y : this->ReferenceHeightValue;
    if (this->LiveWindowMatchesReferenceAspect(liveWidth, liveHeight))
    {
return ([&]() {
auto __ctor_arg_00000200 = static_cast<int32_t>(static_cast<int32_t>(Math::Round(liveWidth)));
auto __ctor_arg_00000201 = static_cast<int32_t>(static_cast<int32_t>(Math::Round(liveHeight)));
return ::int2(__ctor_arg_00000200, __ctor_arg_00000201);
})();    }
const double widthScale = liveWidth / this->ReferenceWidthValue;
const double heightScale = liveHeight / this->ReferenceHeightValue;
const double scale = Math::Min(widthScale, heightScale);
    if (scale <= 0.0)
    {
return ::int2(static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));    }
const int32_t fittedWidth = Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(this->ReferenceWidthValue * scale))));
const int32_t fittedHeight = Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(this->ReferenceHeightValue * scale))));
return ::int2(static_cast<int32_t>(fittedWidth), static_cast<int32_t>(fittedHeight));}

::float2 ReferenceCanvasFitComponent::ResolveCurrentCanvasOrigin(::int2 anchorSpaceSize)
{
::int2 mainWindowSize = Core::Instance->RenderManager3D->MainWindowSize;
const double liveWidth = mainWindowSize.X > 0 ? mainWindowSize.X : this->ReferenceWidthValue;
const double liveHeight = mainWindowSize.Y > 0 ? mainWindowSize.Y : this->ReferenceHeightValue;
const float originX = static_cast<float>(((liveWidth - anchorSpaceSize.X) * 0.5));
const float originY = static_cast<float>(((liveHeight - anchorSpaceSize.Y) * 0.5));
return ::float2(originX, originY);}

