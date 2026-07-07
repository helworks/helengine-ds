#ifdef DrawText
#undef DrawText
#endif
#include "LayoutComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "AnchorSpace.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "float4.hpp"
#include "runtime/native_nullable.hpp"
#include "Component.hpp"
#include "NativeOwnership.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "float2.hpp"
#include "Core.hpp"
#include "runtime/native_event.hpp"
#include "float4x4.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
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
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "LayoutComponent.hpp"
#include "system/math.hpp"
#include "RoundedRectComponent.hpp"
#include "SpriteComponent.hpp"
#include "TextComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "ComboBoxComponent.hpp"
#include "TextBoxComponent.hpp"
#include "CheckBoxComponent.hpp"
#include "system/action.hpp"
#include "ViewportComponent.hpp"
#include "AnimationPlayerComponent.hpp"
#include "ReferenceCanvasFitComponent.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_nullable.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"

uint8_t LayoutComponent::get_AnchorFlags()
{
return this->AnchorFlags;
}

void LayoutComponent::set_AnchorFlags(uint8_t value)
{
this->AnchorFlags = value;
}

::float4 LayoutComponent::get_AnchorDistances()
{
return this->AnchorDistances;
}

void LayoutComponent::set_AnchorDistances(::float4 value)
{
this->AnchorDistances = value;
}

uint8_t LayoutComponent::get_LayoutSpace()
{
return this->LayoutSpaceValue;}

void LayoutComponent::set_LayoutSpace(uint8_t value)
{
    if (this->LayoutSpaceValue != value)
    {
this->LayoutSpaceValue = value;
    if (this->Parent != nullptr)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
    }
}

bool LayoutComponent::get_IsAnchored()
{
return this->AnchorFlags != 0;
}

::AnchorSpace* LayoutComponent::get_AnchorSpace()
{
::int2 currentSize = this->GetAnchorSize();
this->ChildAnchorSpaceValue->Update(currentSize, ::float2(0.0f, 0.0f));
return this->ChildAnchorSpaceValue;}

void LayoutComponent::ComponentAdded(::Entity* entity)
{
Component::ComponentAdded(entity);
this->PublishOwnAnchorBoundsIfNeeded();
    if (this->get_IsAnchored())
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

void LayoutComponent::ComponentRemoved(::Entity* entity)
{
Component::ComponentRemoved(entity);
this->DisableAnchoring();
}

void LayoutComponent::DisableAnchoring()
{
this->DetachFromBoundsProvider();
this->DetachFromWindowResize();
this->set_AnchorFlags(0);
this->set_AnchorDistances(::float4(0.0f, 0.0f, 0.0f, 0.0f));
this->PublishOwnAnchorBoundsIfNeeded();
}

void LayoutComponent::Dispose()
{
delete this->FallbackAnchorSpaceValue;
delete this->ParentAnchorSpaceValue;
delete this->ChildAnchorSpaceValue;
Component::Dispose();
}

void LayoutComponent::EnableAnchoring(bool left, bool right, bool top, bool bottom)
{
    if (!left && !right && !top && !bottom)
    {
this->DisableAnchoring();
return;    }
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("LayoutComponent must be attached before anchoring can be enabled.");
    }
this->RefreshSubscriptions();
::AnchorSpace *anchorSpace = this->GetAnchorSpace();
::int2 anchoredSize = this->GetAnchorSize();
::float3 localPosition = this->Parent->get_LocalPosition();
uint8_t anchorFlags = 0;
::float4 anchorDistances = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (left)
    {
anchorFlags |= LeftAnchorFlag;
anchorDistances.X = localPosition.X - anchorSpace->Origin.X;
    }
    if (right)
    {
anchorFlags |= RightAnchorFlag;
anchorDistances.Y = anchorSpace->Size.X - (localPosition.X - anchorSpace->Origin.X) - anchoredSize.X;
    }
    if (top)
    {
anchorFlags |= TopAnchorFlag;
anchorDistances.Z = localPosition.Y - anchorSpace->Origin.Y;
    }
    if (bottom)
    {
anchorFlags |= BottomAnchorFlag;
anchorDistances.W = anchorSpace->Size.Y - (localPosition.Y - anchorSpace->Origin.Y) - anchoredSize.Y;
    }
this->set_AnchorFlags(anchorFlags);
this->set_AnchorDistances(anchorDistances);
    if (Core::Instance != nullptr && Core::Instance->RenderManager3D != nullptr)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

std::string LayoutComponent::GetAnchorInfo()
{
    if (!this->get_IsAnchored())
    {
return "Not anchored";    }
List<std::string> *anchors = new List<std::string>();
    if ((this->AnchorFlags & LeftAnchorFlag) != 0)
    {
anchors->Add(std::string("Left (") + std::to_string(this->AnchorDistances.X) + std::string("px)"));
    }
    if ((this->AnchorFlags & RightAnchorFlag) != 0)
    {
anchors->Add(std::string("Right (") + std::to_string(this->AnchorDistances.Y) + std::string("px)"));
    }
    if ((this->AnchorFlags & TopAnchorFlag) != 0)
    {
anchors->Add(std::string("Top (") + std::to_string(this->AnchorDistances.Z) + std::string("px)"));
    }
    if ((this->AnchorFlags & BottomAnchorFlag) != 0)
    {
anchors->Add(std::string("Bottom (") + std::to_string(this->AnchorDistances.W) + std::string("px)"));
    }
return String::Concat("Anchored to: ", String::JoinArray(", ", anchors->ToArray()));}

LayoutComponent::LayoutComponent() : AnchorFlags(), AnchorDistances(), AnchorBoundsChanged(), LayoutSpaceValue(), anchorBoundsProvider(), IsSubscribedToWindowResize(), FallbackAnchorSpaceValue(), ParentAnchorSpaceValue(), ChildAnchorSpaceValue(), LastChildAnchorSizeValue()
{
this->LayoutSpaceValue = InheritedLayoutSpace;
this->FallbackAnchorSpaceValue = ([&]() {
auto __ctor_arg_000001EB = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));
auto __ctor_arg_000001EC = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_000001EB, __ctor_arg_000001EC);
})();
this->ParentAnchorSpaceValue = ([&]() {
auto __ctor_arg_000001ED = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));
auto __ctor_arg_000001EE = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_000001ED, __ctor_arg_000001EE);
})();
this->ChildAnchorSpaceValue = ([&]() {
auto __ctor_arg_000001EF = ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));
auto __ctor_arg_000001F0 = ::float2(0.0f, 0.0f);
return new ::AnchorSpace(__ctor_arg_000001EF, __ctor_arg_000001F0);
})();
this->LastChildAnchorSizeValue = ::int2(static_cast<int32_t>(-1), static_cast<int32_t>(-1));
}

void LayoutComponent::ParentEnabledChange(bool newEnabled)
{
Component::ParentEnabledChange(newEnabled);
    if (!this->get_IsAnchored())
    {
this->PublishOwnAnchorBoundsIfNeeded();
return;    }
    if (newEnabled)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
else {
this->DetachFromBoundsProvider();
this->DetachFromWindowResize();
}
}

void LayoutComponent::RefreshAnchoring()
{
    if (!this->get_IsAnchored() || this->Parent == nullptr)
    {
this->PublishOwnAnchorBoundsIfNeeded();
return;    }
this->RefreshSubscriptions();
::AnchorSpace *anchorSpace = this->GetAnchorSpace();
::int2 currentSize = this->GetAnchorSize();
int32_t targetWidth = currentSize.X;
int32_t targetHeight = currentSize.Y;
::float3 localPosition = this->Parent->get_LocalPosition();
const bool hasLeft = (this->AnchorFlags & LeftAnchorFlag) != 0;
const bool hasRight = (this->AnchorFlags & RightAnchorFlag) != 0;
const bool hasTop = (this->AnchorFlags & TopAnchorFlag) != 0;
const bool hasBottom = (this->AnchorFlags & BottomAnchorFlag) != 0;
    if (hasLeft && hasRight)
    {
targetWidth = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(static_cast<int32_t>(Math::Round(anchorSpace->Size.X - this->AnchorDistances.X - this->AnchorDistances.Y))));
localPosition.X = anchorSpace->Origin.X + this->AnchorDistances.X;
    }
else {
    if (hasLeft)
    {
localPosition.X = anchorSpace->Origin.X + this->AnchorDistances.X;
    }
else {
    if (hasRight)
    {
localPosition.X = anchorSpace->Origin.X + anchorSpace->Size.X - this->AnchorDistances.Y - currentSize.X;
    }
}
}
    if (hasTop && hasBottom)
    {
targetHeight = Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(static_cast<int32_t>(Math::Round(anchorSpace->Size.Y - this->AnchorDistances.Z - this->AnchorDistances.W))));
localPosition.Y = anchorSpace->Origin.Y + this->AnchorDistances.Z;
    }
else {
    if (hasTop)
    {
localPosition.Y = anchorSpace->Origin.Y + this->AnchorDistances.Z;
    }
else {
    if (hasBottom)
    {
localPosition.Y = anchorSpace->Origin.Y + anchorSpace->Size.Y - this->AnchorDistances.W - currentSize.Y;
    }
}
}
this->ApplyResolvedSize(::int2(static_cast<int32_t>(targetWidth), static_cast<int32_t>(targetHeight)));
this->Parent->set_LocalPosition(localPosition);
this->RebaseSiblingAnimationPlayers();
this->PublishOwnAnchorBoundsIfNeeded();
}

void LayoutComponent::SetAnchorDistances(Nullable<float> left, Nullable<float> right, Nullable<float> top, Nullable<float> bottom)
{
    if (!left.get_HasValue() && !right.get_HasValue() && !top.get_HasValue() && !bottom.get_HasValue())
    {
this->DisableAnchoring();
return;    }
uint8_t anchorFlags = 0;
::float4 anchorDistances = ::float4(0.0f, 0.0f, 0.0f, 0.0f);
    if (left.get_HasValue())
    {
anchorFlags |= LeftAnchorFlag;
anchorDistances.X = left.get_Value();
    }
    if (right.get_HasValue())
    {
anchorFlags |= RightAnchorFlag;
anchorDistances.Y = right.get_Value();
    }
    if (top.get_HasValue())
    {
anchorFlags |= TopAnchorFlag;
anchorDistances.Z = top.get_Value();
    }
    if (bottom.get_HasValue())
    {
anchorFlags |= BottomAnchorFlag;
anchorDistances.W = bottom.get_Value();
    }
this->set_AnchorFlags(anchorFlags);
this->set_AnchorDistances(anchorDistances);
    if (this->Parent != nullptr && Core::Instance != nullptr && Core::Instance->RenderManager3D != nullptr)
    {
this->RefreshSubscriptions();
this->RefreshAnchoring();
    }
}

::Entity* LayoutComponent::get_Parent()
{
return Component::get_Parent();
}

void LayoutComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool LayoutComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* LayoutComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool LayoutComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void LayoutComponent::ApplyResolvedSize(::int2 resolvedSize)
{
    if (this->Parent == nullptr || this->Parent->get_Components() == nullptr)
    {
return;    }
for (int32_t componentIndex = 0; componentIndex < this->Parent->get_Components()->get_Count(); componentIndex++) {
::Component *component = (*this->Parent->get_Components()).get_Item(static_cast<int32_t>(componentIndex));
    RoundedRectComponent* roundedRectComponent = he_cpp_try_cast<RoundedRectComponent>(component);
    if (roundedRectComponent != nullptr)
    {
roundedRectComponent->set_Size(resolvedSize);
    }
else {
    SpriteComponent* spriteComponent = he_cpp_try_cast<SpriteComponent>(component);
    if (spriteComponent != nullptr)
    {
spriteComponent->set_Size(resolvedSize);
    }
else {
    TextComponent* textComponent = he_cpp_try_cast<TextComponent>(component);
    if (textComponent != nullptr)
    {
textComponent->set_Size(resolvedSize);
    }
else {
    ClipRectComponent* clipRectComponent = he_cpp_try_cast<ClipRectComponent>(component);
    if (clipRectComponent != nullptr)
    {
clipRectComponent->set_Size(resolvedSize);
    }
else {
    InteractableComponent* interactableComponent = he_cpp_try_cast<InteractableComponent>(component);
    if (interactableComponent != nullptr)
    {
interactableComponent->set_Size(resolvedSize);
    }
else {
    ScrollComponent* scrollComponent = he_cpp_try_cast<ScrollComponent>(component);
    if (scrollComponent != nullptr)
    {
scrollComponent->set_Size(resolvedSize);
    }
else {
    ComboBoxComponent* comboBoxComponent = he_cpp_try_cast<ComboBoxComponent>(component);
    if (comboBoxComponent != nullptr)
    {
comboBoxComponent->set_Size(resolvedSize);
    }
else {
    TextBoxComponent* textBoxComponent = he_cpp_try_cast<TextBoxComponent>(component);
    if (textBoxComponent != nullptr)
    {
textBoxComponent->set_Size(resolvedSize);
    }
else {
    CheckBoxComponent* checkBoxComponent = he_cpp_try_cast<CheckBoxComponent>(component);
    if (checkBoxComponent != nullptr)
    {
checkBoxComponent->set_Size(resolvedSize);
    }
}
}
}
}
}
}
}
}
}
}

void LayoutComponent::AttachToWindowResize()
{
    if (this->IsSubscribedToWindowResize)
    {
return;    }
    if (Core::Instance == nullptr || Core::Instance->RenderManager3D == nullptr)
    {
return;    }
Core::Instance->RenderManager3D->WindowResized += Event::Bind(this, static_cast<void (LayoutComponent::*)(intptr_t, int32_t, int32_t)>(&LayoutComponent::HandleWindowResized));
this->IsSubscribedToWindowResize = true;
}

void LayoutComponent::DetachFromBoundsProvider()
{
    if (this->anchorBoundsProvider != nullptr)
    {
this->anchorBoundsProvider->AnchorBoundsChanged -= Event::Bind(this, static_cast<void (LayoutComponent::*)()>(&LayoutComponent::HandleAnchorBoundsChanged));
this->anchorBoundsProvider = nullptr;
    }
}

void LayoutComponent::DetachFromWindowResize()
{
    if (!this->IsSubscribedToWindowResize)
    {
return;    }
Core::Instance->RenderManager3D->WindowResized -= Event::Bind(this, static_cast<void (LayoutComponent::*)(intptr_t, int32_t, int32_t)>(&LayoutComponent::HandleWindowResized));
this->IsSubscribedToWindowResize = false;
}

int32_t LayoutComponent::GetAnchorArea(::int2 size)
{
    if (size.X < 0 || size.Y < 0)
    {
return -1;    }
return size.X * size.Y;}

::int2 LayoutComponent::GetAnchorSize()
{
    if (this->Parent == nullptr)
    {
return ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));    }
::IAnchorSizeProvider *bestProvider = nullptr;
int32_t bestArea = -1;
    IAnchorSizeProvider* parentProvider = he_cpp_try_cast<IAnchorSizeProvider>(this->Parent);
    if (parentProvider != nullptr)
    {
bestProvider = parentProvider;
bestArea = this->GetAnchorArea(parentProvider->get_AnchorSize());
    }
for (int32_t componentIndex = 0; componentIndex < this->Parent->get_Components()->get_Count(); componentIndex++) {
    IAnchorSizeProvider* sizeProvider = he_cpp_try_cast<IAnchorSizeProvider>((*this->Parent->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (sizeProvider != nullptr)
    {
const int32_t area = this->GetAnchorArea(sizeProvider->get_AnchorSize());
    if (area > bestArea)
    {
bestProvider = sizeProvider;
bestArea = area;
    }
    }
}
    if (bestProvider == nullptr)
    {
return ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));    }
return bestProvider->get_AnchorSize();}

::AnchorSpace* LayoutComponent::GetAnchorSpace()
{
    if (this->LayoutSpaceValue == ParentLayoutRectSpace)
    {
    if (this->anchorBoundsProvider != nullptr)
    {
return this->anchorBoundsProvider->get_AnchorSpace();    }
::int2 parentSize = this->ResolveImmediateParentAnchorSize();
this->ParentAnchorSpaceValue->Update(parentSize, ::float2(0.0f, 0.0f));
return this->ParentAnchorSpaceValue;    }
ViewportComponent* viewportComponent = he_cpp_try_cast<ViewportComponent>(this->anchorBoundsProvider);
    if (this->LayoutSpaceValue == CameraViewportLayoutSpace && viewportComponent != nullptr)
    {
return viewportComponent->get_ViewportAnchorSpace();    }
    if (this->anchorBoundsProvider != nullptr)
    {
return this->anchorBoundsProvider->get_AnchorSpace();    }
this->FallbackAnchorSpaceValue->Update(Core::Instance->RenderManager3D->MainWindowSize, ::float2(0.0f, 0.0f));
return this->FallbackAnchorSpaceValue;}

void LayoutComponent::HandleAnchorBoundsChanged()
{
this->RefreshAnchoring();
}

void LayoutComponent::HandleWindowResized(intptr_t handle, int32_t newWidth, int32_t newHeight)
{
this->RefreshAnchoring();
}

void LayoutComponent::PublishOwnAnchorBoundsIfNeeded()
{
::int2 currentSize = this->GetAnchorSize();
    if (currentSize.X == this->LastChildAnchorSizeValue.X && currentSize.Y == this->LastChildAnchorSizeValue.Y)
    {
return;    }
this->LastChildAnchorSizeValue = currentSize;
this->ChildAnchorSpaceValue->Update(currentSize, ::float2(0.0f, 0.0f));
    if (true)
    {
this->AnchorBoundsChanged.Invoke();
    }
}

void LayoutComponent::RebaseSiblingAnimationPlayers()
{
    if (this->Parent == nullptr || this->Parent->get_Components() == nullptr)
    {
return;    }
for (int32_t componentIndex = 0; componentIndex < this->Parent->get_Components()->get_Count(); componentIndex++) {
    AnimationPlayerComponent* animationPlayerComponent = he_cpp_try_cast<AnimationPlayerComponent>((*this->Parent->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (animationPlayerComponent != nullptr)
    {
animationPlayerComponent->RebaseCurrentPoseToLocalTransform();
    }
}
}

void LayoutComponent::RefreshSubscriptions()
{
::IAnchorBoundsProvider *newProvider = this->ResolveAnchorBoundsProvider();
    if (!(this->anchorBoundsProvider == newProvider))
    {
this->DetachFromBoundsProvider();
this->anchorBoundsProvider = newProvider;
    if (this->anchorBoundsProvider != nullptr)
    {
this->anchorBoundsProvider->AnchorBoundsChanged += Event::Bind(this, static_cast<void (LayoutComponent::*)()>(&LayoutComponent::HandleAnchorBoundsChanged));
    }
    }
    if (this->anchorBoundsProvider == nullptr && this->LayoutSpaceValue != ParentLayoutRectSpace)
    {
this->AttachToWindowResize();
    }
else {
this->DetachFromWindowResize();
}
}

::IAnchorBoundsProvider* LayoutComponent::ResolveAncestorReferenceCanvasProvider()
{
::Entity *current = this->Parent != nullptr ? this->Parent->Parent : nullptr;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    ReferenceCanvasFitComponent* provider = he_cpp_try_cast<ReferenceCanvasFitComponent>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (provider != nullptr)
    {
return provider;    }
}
    }
current = current->Parent;
}
return nullptr;}

::IAnchorBoundsProvider* LayoutComponent::ResolveAncestorViewportProvider()
{
::Entity *current = this->Parent != nullptr ? this->Parent->Parent : nullptr;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < current->get_Components()->get_Count(); componentIndex++) {
    ViewportComponent* provider = he_cpp_try_cast<ViewportComponent>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (provider != nullptr)
    {
return provider;    }
}
    }
current = current->Parent;
}
return nullptr;}

::IAnchorBoundsProvider* LayoutComponent::ResolveAnchorBoundsProvider()
{
    if (this->Parent == nullptr)
    {
return nullptr;    }
    if (this->LayoutSpaceValue == ParentLayoutRectSpace)
    {
return this->ResolveImmediateParentLayoutProvider();    }
    if (this->LayoutSpaceValue == ReferenceCanvasLayoutSpace)
    {
return this->ResolveAncestorReferenceCanvasProvider();    }
    if (this->LayoutSpaceValue == CameraViewportLayoutSpace)
    {
return this->ResolveAncestorViewportProvider();    }
return this->ResolveInheritedBoundsProvider();}

::int2 LayoutComponent::ResolveImmediateParentAnchorSize()
{
::Entity *parentEntity = this->Parent != nullptr ? this->Parent->Parent : nullptr;
    if (parentEntity == nullptr)
    {
return ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));    }
    IAnchorSizeProvider* parentProvider = he_cpp_try_cast<IAnchorSizeProvider>(parentEntity);
    if (parentProvider != nullptr)
    {
return parentProvider->get_AnchorSize();    }
    if (parentEntity->get_Components() == nullptr)
    {
return ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));    }
::IAnchorSizeProvider *bestProvider = nullptr;
int32_t bestArea = -1;
for (int32_t componentIndex = 0; componentIndex < parentEntity->get_Components()->get_Count(); componentIndex++) {
    IAnchorSizeProvider* sizeProvider = he_cpp_try_cast<IAnchorSizeProvider>((*parentEntity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (sizeProvider != nullptr)
    {
const int32_t area = this->GetAnchorArea(sizeProvider->get_AnchorSize());
    if (area > bestArea)
    {
bestProvider = sizeProvider;
bestArea = area;
    }
    }
}
return bestProvider != nullptr ? bestProvider->get_AnchorSize() : ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0));}

::IAnchorBoundsProvider* LayoutComponent::ResolveImmediateParentLayoutProvider()
{
::Entity *parentEntity = this->Parent != nullptr ? this->Parent->Parent : nullptr;
    if (parentEntity == nullptr || parentEntity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < parentEntity->get_Components()->get_Count(); componentIndex++) {
    LayoutComponent* layoutComponent = he_cpp_try_cast<LayoutComponent>((*parentEntity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (layoutComponent != nullptr)
    {
return layoutComponent;    }
}
    IAnchorBoundsProvider* provider = he_cpp_try_cast<IAnchorBoundsProvider>(parentEntity);
    if (provider != nullptr)
    {
return provider;    }
return nullptr;}

::IAnchorBoundsProvider* LayoutComponent::ResolveInheritedBoundsProvider()
{
::Entity *current = this->Parent != nullptr ? this->Parent->Parent : nullptr;
while (current != nullptr) {
    if (current->get_Components() != nullptr)
    {
for (int32_t componentIndex = current->get_Components()->get_Count() - 1; componentIndex >= 0; componentIndex--) {
    IAnchorBoundsProvider* componentProvider = he_cpp_try_cast<IAnchorBoundsProvider>((*current->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (componentProvider != nullptr)
    {
return componentProvider;    }
}
    }
    IAnchorBoundsProvider* provider = he_cpp_try_cast<IAnchorBoundsProvider>(current);
    if (provider != nullptr)
    {
return provider;    }
current = current->Parent;
}
return nullptr;}

