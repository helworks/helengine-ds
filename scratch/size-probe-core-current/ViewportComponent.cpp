#ifdef DrawText
#undef DrawText
#endif
#include "ViewportComponent.hpp"
#include "UpdateComponent.hpp"
#include "NativeOwnership.hpp"
#include "CameraComponent.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "Core.hpp"
#include "RenderManager3D.hpp"
#include "runtime/native_exceptions.hpp"
#include "Entity.hpp"
#include "runtime/native_list.hpp"
#include "float2.hpp"
#include "AnchorSpace.hpp"
#include "system/math.hpp"
#include "SceneCanvasProfile.hpp"
#include "ViewportLayoutSnapshot.hpp"
#include "CameraViewportResolver.hpp"
#include "runtime/native_event.hpp"
#include "runtime/array.hpp"
#include "RenderTarget.hpp"
#include "CameraClearSettings.hpp"
#include "CameraRenderSettings.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "RenderList2D.hpp"
#include "RenderList3D.hpp"
#include "float3.hpp"
#include "runtime/native_string.hpp"
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
#include "RendererBackendCapabilityProfile.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "LayoutComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "SpriteComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "ViewportComponent.hpp"
#include "system/action.hpp"
#include "RuntimeTexture.hpp"
#include "ReferenceCanvasFitComponent.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t ViewportComponent::get_BindingMode()
{
return this->BindingModeValue;}

void ViewportComponent::set_BindingMode(uint8_t value)
{
    if (this->BindingModeValue != value)
    {
this->BindingModeValue = value;
this->RefreshSubscriptions();
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

::int2 ViewportComponent::get_FixedSize()
{
return this->FixedSizeValue;}

void ViewportComponent::set_FixedSize(::int2 value)
{
    if (this->FixedSizeValue.X != value.X || this->FixedSizeValue.Y != value.Y)
    {
this->FixedSizeValue = value;
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

uint8_t ViewportComponent::get_ScalingMode()
{
return this->ScalingModeValue;}

void ViewportComponent::set_ScalingMode(uint8_t value)
{
    if (this->ScalingModeValue != value)
    {
this->ScalingModeValue = value;
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

int32_t ViewportComponent::get_ReferenceWidth()
{
return this->ReferenceWidthValue;}

void ViewportComponent::set_ReferenceWidth(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_0000021F = "value";
auto __ctor_arg_00000220 = "Reference width must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_0000021F, __ctor_arg_00000220);
})();
    }
    if (this->ReferenceWidthValue != value)
    {
this->ReferenceWidthValue = value;
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

int32_t ViewportComponent::get_ReferenceHeight()
{
return this->ReferenceHeightValue;}

void ViewportComponent::set_ReferenceHeight(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_00000221 = "value";
auto __ctor_arg_00000222 = "Reference height must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_00000221, __ctor_arg_00000222);
})();
    }
    if (this->ReferenceHeightValue != value)
    {
this->ReferenceHeightValue = value;
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

::CameraComponent* ViewportComponent::get_BoundCameraComponent()
{
return this->ExplicitBoundCameraComponentValue;}

void ViewportComponent::set_BoundCameraComponent(::CameraComponent* value)
{
    if (!(this->ExplicitBoundCameraComponentValue == value))
    {
this->ExplicitBoundCameraComponentValue = value;
this->RefreshSubscriptions();
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
}

::AnchorSpace* ViewportComponent::get_AnchorSpace()
{
this->RefreshSubscriptions();
    if (this->ScalingModeValue == ReferenceCanvasScalingMode)
    {
return this->CurrentAnchorSpaceValue;    }
this->CurrentAnchorSpaceValue->Update(this->ResolveAnchorBounds(), ::float2(0.0f, 0.0f));
return this->CurrentAnchorSpaceValue;}

::AnchorSpace* ViewportComponent::get_ViewportAnchorSpace()
{
this->RefreshSubscriptions();
this->CurrentViewportAnchorSpaceValue->Update(this->ResolveAnchorBounds(), this->ResolveViewportAnchorOrigin());
return this->CurrentViewportAnchorSpaceValue;}

::float4 ViewportComponent::get_ResolvedViewportBounds()
{
this->RefreshSubscriptions();
return this->ResolveViewportBounds();}

::int2 ViewportComponent::get_ResolvedViewportSize()
{
::float4 viewport = this->get_ResolvedViewportBounds();
return ([&]() {
auto __ctor_arg_00000223 = static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(viewport.Z)))));
auto __ctor_arg_00000224 = static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(viewport.W)))));
return ::int2(__ctor_arg_00000223, __ctor_arg_00000224);
})();}

void ViewportComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
this->RefreshSubscriptions();
this->RebuildSnapshots();
this->PendingScaleApplyValue = true;
}

void ViewportComponent::ComponentRemoved(::Entity* entity)
{
UpdateComponent::ComponentRemoved(entity);
this->DetachFromCamera();
this->DetachFromWindowResize();
this->ReleaseLayoutSnapshotItems();
this->SnapshotEntityCountValue = 0;
this->PendingScaleApplyValue = false;
}

void ViewportComponent::Dispose()
{
this->DetachFromCamera();
this->DetachFromWindowResize();
this->ReleaseLayoutSnapshots();
delete this->CurrentAnchorSpaceValue;
delete this->CurrentViewportAnchorSpaceValue;
this->ActiveCameraComponentValue = nullptr;
this->ExplicitBoundCameraComponentValue = nullptr;
this->CurrentAnchorSpaceValue = nullptr;
this->CurrentViewportAnchorSpaceValue = nullptr;
UpdateComponent::Dispose();
}

::CameraComponent* ViewportComponent::GetBoundCameraComponent()
{
this->RefreshSubscriptions();
return this->ResolveBoundCameraComponent();}

void ViewportComponent::ParentEnabledChange(bool newEnabled)
{
UpdateComponent::ParentEnabledChange(newEnabled);
    if (newEnabled)
    {
this->RefreshSubscriptions();
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
    }
else {
this->DetachFromCamera();
this->DetachFromWindowResize();
}
}

void ViewportComponent::Update()
{
UpdateComponent::Update();
this->RefreshSubscriptions();
    if (this->Parent == nullptr || this->ScalingModeValue != ReferenceCanvasScalingMode)
    {
return;    }
const int32_t currentEntityCount = this->CountEntitiesRecursive(this->Parent);
    if (currentEntityCount != this->SnapshotEntityCountValue)
    {
this->RebuildSnapshots();
this->PendingScaleApplyValue = true;
    }
    if (this->PendingScaleApplyValue)
    {
this->ApplyCurrentScale();
this->PendingScaleApplyValue = false;
    }
}

ViewportComponent::ViewportComponent() : AnchorBoundsChanged(), BindingModeValue(), FixedSizeValue(), ActiveCameraComponentValue(), ExplicitBoundCameraComponentValue(), ScalingModeValue(), ReferenceWidthValue(0), ReferenceHeightValue(0), LayoutSnapshotsValue(), IsSubscribedToWindowResizeValue(), PendingScaleApplyValue(), SnapshotEntityCountValue(0), CurrentAnchorSpaceValue(), CurrentViewportAnchorSpaceValue(), CurrentCanvasOriginValue()
{
this->BindingModeValue = ScreenBindingMode;
this->FixedSizeValue = ::int2(static_cast<int32_t>(SceneCanvasProfile::DefaultWidth), static_cast<int32_t>(SceneCanvasProfile::DefaultHeight));
this->ReferenceWidthValue = SceneCanvasProfile::DefaultWidth;
this->ReferenceHeightValue = SceneCanvasProfile::DefaultHeight;
this->LayoutSnapshotsValue = new List<::ViewportLayoutSnapshot*>();
this->CurrentAnchorSpaceValue = ([&]() {
auto __ctor_arg_00000225 = ::int2(static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));
auto __ctor_arg_00000226 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000225, __ctor_arg_00000226);
})();
this->CurrentViewportAnchorSpaceValue = ([&]() {
auto __ctor_arg_00000227 = ::int2(static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));
auto __ctor_arg_00000228 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_00000227, __ctor_arg_00000228);
})();
this->CurrentCanvasOriginValue = ::float2(0.0f, 0.0f);
}

uint8_t ViewportComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void ViewportComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* ViewportComponent::get_Parent()
{
return Component::get_Parent();
}

void ViewportComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ViewportComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ViewportComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ViewportComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void ViewportComponent::ApplyCurrentScale()
{
    if (this->Parent == nullptr || this->LayoutSnapshotsValue->get_Count() == 0)
    {
return;    }
::int2 resolvedAnchorSpaceSize = this->ResolveCurrentAnchorSpaceSize();
::float2 resolvedAnchorSpaceOrigin = ::float2(0.0f, 0.0f);
::float2 resolvedCanvasOrigin = this->ResolveCurrentCanvasOrigin(resolvedAnchorSpaceSize);
const bool anchorSpaceChanged = this->DidAnchorSpaceChange(this->CurrentAnchorSpaceValue, resolvedAnchorSpaceSize, resolvedAnchorSpaceOrigin) || this->DidCanvasOriginChange(this->CurrentCanvasOriginValue, resolvedCanvasOrigin);
this->CurrentAnchorSpaceValue->Update(resolvedAnchorSpaceSize, resolvedAnchorSpaceOrigin);
this->CurrentCanvasOriginValue = resolvedCanvasOrigin;
for (int32_t snapshotIndex = 0; snapshotIndex < this->LayoutSnapshotsValue->get_Count(); snapshotIndex++) {
(*this->LayoutSnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex))->Apply(this->CurrentAnchorSpaceValue, resolvedCanvasOrigin, static_cast<int32_t>(this->ReferenceWidthValue), static_cast<int32_t>(this->ReferenceHeightValue));
}
for (int32_t snapshotIndex = 0; snapshotIndex < this->LayoutSnapshotsValue->get_Count(); snapshotIndex++) {
(*this->LayoutSnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex))->RefreshAnchoring();
}
    if (anchorSpaceChanged)
    {
this->RaiseAnchorBoundsChanged();
    }
}

void ViewportComponent::AttachToCamera()
{
    if (this->ActiveCameraComponentValue == nullptr)
    {
return;    }
this->ActiveCameraComponentValue->ViewportChanged += Event::Bind(this, static_cast<void (ViewportComponent::*)()>(&ViewportComponent::HandleCameraViewportChanged));
}

void ViewportComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResizeValue)
    {
return;    }
Core::Instance->RenderManager3D->WindowResized += Event::Bind(this, static_cast<void (ViewportComponent::*)(intptr_t, int32_t, int32_t)>(&ViewportComponent::HandleWindowResized));
this->IsSubscribedToWindowResizeValue = true;
}

void ViewportComponent::CaptureSnapshotsRecursive(::Entity* entity, bool isRootEntity)
{
this->LayoutSnapshotsValue->Add(new ::ViewportLayoutSnapshot(entity, isRootEntity));
    if (entity->get_Children() == nullptr)
    {
return;    }
for (int32_t childIndex = 0; childIndex < entity->get_Children()->get_Count(); childIndex++) {
this->CaptureSnapshotsRecursive((*entity->get_Children()).get_Item(static_cast<int32_t>(childIndex)), false);
}
}

int32_t ViewportComponent::CountEntitiesRecursive(::Entity* entity)
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

void ViewportComponent::DetachFromCamera()
{
    if (this->ActiveCameraComponentValue != nullptr)
    {
this->ActiveCameraComponentValue->ViewportChanged -= Event::Bind(this, static_cast<void (ViewportComponent::*)()>(&ViewportComponent::HandleCameraViewportChanged));
this->ActiveCameraComponentValue = nullptr;
    }
}

void ViewportComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResizeValue)
    {
return;    }
Core::Instance->RenderManager3D->WindowResized -= Event::Bind(this, static_cast<void (ViewportComponent::*)(intptr_t, int32_t, int32_t)>(&ViewportComponent::HandleWindowResized));
this->IsSubscribedToWindowResizeValue = false;
}

bool ViewportComponent::DidAnchorSpaceChange(::AnchorSpace* currentAnchorSpace, ::int2 resolvedAnchorSpaceSize, ::float2 resolvedAnchorSpaceOrigin)
{
    if (currentAnchorSpace == nullptr)
    {
return true;    }
return currentAnchorSpace->Size.X != resolvedAnchorSpaceSize.X || currentAnchorSpace->Size.Y != resolvedAnchorSpaceSize.Y || currentAnchorSpace->Origin.X != resolvedAnchorSpaceOrigin.X || currentAnchorSpace->Origin.Y != resolvedAnchorSpaceOrigin.Y;}

bool ViewportComponent::DidCanvasOriginChange(::float2 currentCanvasOrigin, ::float2 resolvedCanvasOrigin)
{
return currentCanvasOrigin.X != resolvedCanvasOrigin.X || currentCanvasOrigin.Y != resolvedCanvasOrigin.Y;}

void ViewportComponent::HandleCameraViewportChanged()
{
this->RaiseAnchorBoundsChanged();
}

void ViewportComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->PendingScaleApplyValue = true;
this->RaiseAnchorBoundsChanged();
}

bool ViewportComponent::LiveViewportMatchesReferenceAspect(double liveWidth, double liveHeight)
{
const double expectedWidth = liveHeight * this->ReferenceWidthValue / this->ReferenceHeightValue;
const double expectedHeight = liveWidth * this->ReferenceHeightValue / this->ReferenceWidthValue;
return Math::Abs(liveWidth - expectedWidth) <= 0.5 || Math::Abs(liveHeight - expectedHeight) <= 0.5;}

void ViewportComponent::RaiseAnchorBoundsChanged()
{
    if (true)
    {
this->AnchorBoundsChanged.Invoke();
    }
}

void ViewportComponent::RebuildSnapshots()
{
this->ReleaseLayoutSnapshotItems();
    if (this->Parent == nullptr)
    {
this->SnapshotEntityCountValue = 0;
return;    }
this->CaptureSnapshotsRecursive(this->Parent, true);
this->SnapshotEntityCountValue = this->LayoutSnapshotsValue->get_Count();
}

void ViewportComponent::RefreshSubscriptions()
{
    if (this->BindingModeValue == ScreenBindingMode)
    {
this->DetachFromCamera();
this->AttachToWindowResize();
    }
else {
    if (this->BindingModeValue == AncestorCameraBindingMode)
    {
this->DetachFromWindowResize();
::CameraComponent *nextCameraComponent = this->ResolveAncestorCameraComponent();
    if (!(this->ActiveCameraComponentValue == nextCameraComponent))
    {
this->DetachFromCamera();
this->ActiveCameraComponentValue = nextCameraComponent;
this->AttachToCamera();
    }
    }
else {
    if (this->BindingModeValue == ExplicitCameraBindingMode)
    {
this->DetachFromWindowResize();
    if (!(this->ActiveCameraComponentValue == ExplicitBoundCameraComponentValue))
    {
this->DetachFromCamera();
this->ActiveCameraComponentValue = this->ExplicitBoundCameraComponentValue;
this->AttachToCamera();
    }
    }
else {
this->DetachFromCamera();
this->DetachFromWindowResize();
}
}
}
}

void ViewportComponent::ReleaseLayoutSnapshotItems()
{
for (int32_t snapshotIndex = 0; snapshotIndex < this->LayoutSnapshotsValue->get_Count(); snapshotIndex++) {
delete (*this->LayoutSnapshotsValue).get_Item(static_cast<int32_t>(snapshotIndex));
}
this->LayoutSnapshotsValue->Clear();
}

void ViewportComponent::ReleaseLayoutSnapshots()
{
this->ReleaseLayoutSnapshotItems();
delete this->LayoutSnapshotsValue;
}

::CameraComponent* ViewportComponent::ResolveAncestorCameraComponent()
{
::Entity *current = this->Parent;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t index = 0; index < current->get_Components()->get_Count(); index++) {
    CameraComponent* cameraComponent = he_cpp_try_cast<CameraComponent>((*current->get_Components()).get_Item(static_cast<int32_t>(index)));
    if (cameraComponent != nullptr)
    {
return cameraComponent;    }
}
    }
current = current->Parent;
}
return nullptr;}

::int2 ViewportComponent::ResolveAnchorBounds()
{
::float4 viewport = this->ResolveViewportBounds();
return ([&]() {
auto __ctor_arg_00000229 = static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(viewport.Z)))));
auto __ctor_arg_0000022A = static_cast<int32_t>(Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(viewport.W)))));
return ::int2(__ctor_arg_00000229, __ctor_arg_0000022A);
})();}

::CameraComponent* ViewportComponent::ResolveBoundCameraComponent()
{
    if (this->BindingModeValue == ExplicitCameraBindingMode)
    {
return this->ExplicitBoundCameraComponentValue;    }
    if (this->BindingModeValue == AncestorCameraBindingMode)
    {
return this->ResolveAncestorCameraComponent();    }
return nullptr;}

::float4 ViewportComponent::ResolveCameraViewportBounds(::CameraComponent* cameraComponent)
{
    if (cameraComponent == nullptr)
    {
throw new ArgumentNullException("cameraComponent");
    }
    if (cameraComponent->RenderTarget != nullptr && cameraComponent->RenderTarget->Width > 0 && cameraComponent->RenderTarget->Height > 0)
    {
return ::float4(0.0f, 0.0f, cameraComponent->RenderTarget->Width, cameraComponent->RenderTarget->Height);    }
::float4 viewport = cameraComponent->get_Viewport();
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
return viewport;    }
::int2 mainWindowSize = Core::Instance->RenderManager3D->MainWindowSize;
    if (mainWindowSize.X <= 0 || mainWindowSize.Y <= 0)
    {
return viewport;    }
return CameraViewportResolver::ResolveViewport(viewport, mainWindowSize.X, mainWindowSize.Y);}

::int2 ViewportComponent::ResolveCurrentAnchorSpaceSize()
{
::int2 viewportBounds = this->ResolveAnchorBounds();
const double liveWidth = viewportBounds.X > 0 ? viewportBounds.X : this->ReferenceWidthValue;
const double liveHeight = viewportBounds.Y > 0 ? viewportBounds.Y : this->ReferenceHeightValue;
    if (this->LiveViewportMatchesReferenceAspect(liveWidth, liveHeight))
    {
return ([&]() {
auto __ctor_arg_0000022B = static_cast<int32_t>(static_cast<int32_t>(Math::Round(liveWidth)));
auto __ctor_arg_0000022C = static_cast<int32_t>(static_cast<int32_t>(Math::Round(liveHeight)));
return ::int2(__ctor_arg_0000022B, __ctor_arg_0000022C);
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

::float2 ViewportComponent::ResolveCurrentCanvasOrigin(::int2 anchorSpaceSize)
{
::int2 viewportBounds = this->ResolveAnchorBounds();
const double liveWidth = viewportBounds.X > 0 ? viewportBounds.X : this->ReferenceWidthValue;
const double liveHeight = viewportBounds.Y > 0 ? viewportBounds.Y : this->ReferenceHeightValue;
const float originX = static_cast<float>(((liveWidth - anchorSpaceSize.X) * 0.5));
const float originY = static_cast<float>(((liveHeight - anchorSpaceSize.Y) * 0.5));
return ::float2(originX, originY);}

::float2 ViewportComponent::ResolveViewportAnchorOrigin()
{
    if (this->ScalingModeValue == ReferenceCanvasScalingMode)
    {
return ::float2(-this->CurrentCanvasOriginValue.X, -this->CurrentCanvasOriginValue.Y);    }
    if (this->Parent == nullptr || this->Parent->get_Components() == nullptr)
    {
return ::float2(0.0f, 0.0f);    }
for (int32_t componentIndex = 0; componentIndex < this->Parent->get_Components()->get_Count(); componentIndex++) {
    ReferenceCanvasFitComponent* referenceCanvasFitComponent = he_cpp_try_cast<ReferenceCanvasFitComponent>((*this->Parent->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (referenceCanvasFitComponent != nullptr)
    {
return referenceCanvasFitComponent->get_ViewportAnchorOrigin();    }
}
return ::float2(0.0f, 0.0f);}

::float4 ViewportComponent::ResolveViewportBounds()
{
    if (this->BindingModeValue == ScreenBindingMode)
    {
::int2 screenSize = Core::Instance->RenderManager3D->MainWindowSize;
    if (screenSize.X > 0 && screenSize.Y > 0)
    {
return ::float4(0.0f, 0.0f, screenSize.X, screenSize.Y);    }
return ::float4(0.0f, 0.0f, this->FixedSizeValue.X, this->FixedSizeValue.Y);    }
    if (this->BindingModeValue == AncestorCameraBindingMode || this->BindingModeValue == ExplicitCameraBindingMode)
    {
::CameraComponent *cameraComponent = this->ResolveBoundCameraComponent();
    if (cameraComponent != nullptr)
    {
return this->ResolveCameraViewportBounds(cameraComponent);    }
return ::float4(0.0f, 0.0f, this->FixedSizeValue.X, this->FixedSizeValue.Y);    }
return ::float4(0.0f, 0.0f, this->FixedSizeValue.X, this->FixedSizeValue.Y);}

