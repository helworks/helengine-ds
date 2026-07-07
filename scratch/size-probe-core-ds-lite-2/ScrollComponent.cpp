#ifdef DrawText
#undef DrawText
#endif
#include "ScrollComponent.hpp"
#include "float4.hpp"
#include "Core.hpp"
#include "InputSystem.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "runtime/native_exceptions.hpp"
#include "int2.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_string.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "FontAsset.hpp"
#include "RenderManager2D.hpp"
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
#include "InputFrameState.hpp"
#include "runtime/native_list.hpp"
#include "InputBinding.hpp"
#include "InputActionState.hpp"
#include "MouseState.hpp"
#include "KeyboardState.hpp"
#include "system/action.hpp"
#include "InputContextId.hpp"
#include "InputActionId.hpp"
#include "InputGamepadState.hpp"
#include "ButtonState.hpp"
#include "InputPointerState.hpp"
#include "InputTextState.hpp"
#include "Keys.hpp"
#include "InputGamepadButton.hpp"
#include "float2.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "ScrollComponent.hpp"
#include "system/math.hpp"
#include "ClipRectComponent.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "system/diagnostics/stopwatch.hpp"

ScrollComponent::ScrollComponent() : ScrollOffsetChanged(), ScrollOffset(0), SizeValue(), ItemCountValue(0), ItemExtentValue(1), VisibleItemCountValue(0), ScrollStepCountValue(1), WheelNotchSizeValue(StandardWheelNotch), RequiresPointerInsideValue(true), ContentRootValue(), ClipOriginEntityValue()
{
}

::int2 ScrollComponent::get_Size()
{
return this->ResolveViewportSize();}

void ScrollComponent::set_Size(::int2 value)
{
    if (value.X < 0 || value.Y < 0)
    {
throw ([&]() {
auto __ctor_arg_000001F9 = "value";
auto __ctor_arg_000001FA = "Scroll viewport size must not be negative.";
return new ArgumentOutOfRangeException(__ctor_arg_000001F9, __ctor_arg_000001FA);
})();
    }
this->SizeValue = value;
}

int32_t ScrollComponent::get_ItemCount()
{
return this->ItemCountValue;}

void ScrollComponent::set_ItemCount(int32_t value)
{
    if (value < 0)
    {
throw ([&]() {
auto __ctor_arg_000001FB = "value";
auto __ctor_arg_000001FC = "Item count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FB, __ctor_arg_000001FC);
})();
    }
this->ItemCountValue = value;
this->ClampScrollOffset();
}

int32_t ScrollComponent::get_VisibleItemCount()
{
return this->GetVisibleItemCount();}

void ScrollComponent::set_VisibleItemCount(int32_t value)
{
    if (value < 0)
    {
throw ([&]() {
auto __ctor_arg_000001FD = "value";
auto __ctor_arg_000001FE = "Visible item count must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FD, __ctor_arg_000001FE);
})();
    }
this->VisibleItemCountValue = value;
this->ClampScrollOffset();
}

bool ScrollComponent::get_UsesAutomaticVisibleItemCount()
{
return this->VisibleItemCountValue < 1;}

int32_t ScrollComponent::get_ItemExtent()
{
return this->ItemExtentValue;}

void ScrollComponent::set_ItemExtent(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_000001FF = "value";
auto __ctor_arg_00000200 = "Item extent must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_000001FF, __ctor_arg_00000200);
})();
    }
this->ItemExtentValue = value;
this->ClampScrollOffset();
this->ApplyContentRootOffset();
}

int32_t ScrollComponent::get_MaximumScrollOffset()
{
return Math::Max(static_cast<int32_t>(0), static_cast<int32_t>(this->ItemCountValue - this->GetVisibleItemCount()));}

int32_t ScrollComponent::get_ScrollOffset()
{
return this->ScrollOffset;
}

void ScrollComponent::set_ScrollOffset(int32_t value)
{
this->ScrollOffset = value;
}

::Entity* ScrollComponent::get_ContentRoot()
{
return this->ContentRootValue;}

void ScrollComponent::set_ContentRoot(::Entity* value)
{
this->ContentRootValue = value;
this->ApplyContentRootOffset();
}

::Entity* ScrollComponent::get_ClipOriginEntity()
{
return this->ClipOriginEntityValue;}

void ScrollComponent::set_ClipOriginEntity(::Entity* value)
{
this->ClipOriginEntityValue = value;
}

int32_t ScrollComponent::get_ScrollStepCount()
{
return this->ScrollStepCountValue;}

void ScrollComponent::set_ScrollStepCount(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_00000201 = "value";
auto __ctor_arg_00000202 = "Scroll step count must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_00000201, __ctor_arg_00000202);
})();
    }
this->ScrollStepCountValue = value;
}

int32_t ScrollComponent::get_WheelNotchSize()
{
return this->WheelNotchSizeValue;}

void ScrollComponent::set_WheelNotchSize(int32_t value)
{
    if (value < 1)
    {
throw ([&]() {
auto __ctor_arg_00000203 = "value";
auto __ctor_arg_00000204 = "Wheel notch size must be at least one.";
return new ArgumentOutOfRangeException(__ctor_arg_00000203, __ctor_arg_00000204);
})();
    }
this->WheelNotchSizeValue = value;
}

bool ScrollComponent::get_RequiresPointerInside()
{
return this->RequiresPointerInsideValue;}

void ScrollComponent::set_RequiresPointerInside(bool value)
{
this->RequiresPointerInsideValue = value;
}

void ScrollComponent::ClampScrollOffset()
{
this->SetScrollOffset(static_cast<int32_t>(this->ScrollOffset), false);
}

bool ScrollComponent::ContainsScreenPoint(int32_t x, int32_t y)
{
    if (this->Parent == nullptr)
    {
return false;    }
::float4 viewportRect = this->GetClipRect();
return x >= viewportRect.X && x < viewportRect.X + viewportRect.Z && y >= viewportRect.Y && y < viewportRect.Y + viewportRect.W;}

::float4 ScrollComponent::GetClipRect()
{
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("Scroll components require an attached parent entity.");
    }
::Entity *clipOriginEntity = ([&]() {
::Entity* __coalesce_00000205 = this->ClipOriginEntityValue;
return __coalesce_00000205 != nullptr ? __coalesce_00000205 : this->Parent;
})();
::float3 origin = clipOriginEntity->get_Position();
::int2 viewportSize = this->ResolveViewportSize();
return ::float4(origin.X, origin.Y, viewportSize.X, viewportSize.Y);}

void ScrollComponent::ResetScrollOffset()
{
this->SetScrollOffset(static_cast<int32_t>(0), false);
}

bool ScrollComponent::ScrollTo(int32_t scrollOffset)
{
return this->SetScrollOffset(static_cast<int32_t>(scrollOffset), true);}

bool ScrollComponent::TryApplyWheelInput()
{
    if (this->Parent == nullptr)
    {
return false;    }
    if (this->get_MaximumScrollOffset() <= 0)
    {
return false;    }
    if (this->RequiresPointerInsideValue && !this->ContainsScreenPoint(static_cast<int32_t>(Core::Instance->Input->GetMouseX()), static_cast<int32_t>(Core::Instance->Input->GetMouseY())))
    {
return false;    }
const int32_t wheelDelta = Core::Instance->Input->GetMouseScrollWheelDelta();
    if (wheelDelta == 0)
    {
return false;    }
int32_t scrollSteps = wheelDelta / this->WheelNotchSizeValue;
    if (scrollSteps == 0)
    {
scrollSteps = wheelDelta > 0 ? 1 : -1;
    }
scrollSteps *= this->ScrollStepCountValue;
const int32_t nextOffset = this->ScrollOffset - scrollSteps;
return this->SetScrollOffset(static_cast<int32_t>(nextOffset), true);}

void ScrollComponent::Update()
{
this->TryApplyWheelInput();
}

::Entity* ScrollComponent::get_Parent()
{
return this->Component::get_Parent();
}

uint8_t ScrollComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void ScrollComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

void ScrollComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ScrollComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ScrollComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ScrollComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void ScrollComponent::ApplyContentRootOffset()
{
    if (this->ContentRootValue == nullptr)
    {
return;    }
::float3 position = this->ContentRootValue->get_LocalPosition();
this->ContentRootValue->set_LocalPosition(::float3(position.X, -(this->ScrollOffset * this->ItemExtentValue), position.Z));
}

int32_t ScrollComponent::ClampOffset(int32_t scrollOffset)
{
const int32_t maxOffset = this->get_MaximumScrollOffset();
    if (scrollOffset < 0)
    {
return 0;    }
    if (scrollOffset > maxOffset)
    {
return maxOffset;    }
return scrollOffset;}

int32_t ScrollComponent::GetVisibleItemCount()
{
    if (this->VisibleItemCountValue > 0)
    {
return this->VisibleItemCountValue;    }
const int32_t extent = this->ItemExtentValue;
    if (extent <= 0)
    {
return 1;    }
::int2 viewportSize = this->ResolveViewportSize();
return Math::Max(static_cast<int32_t>(1), static_cast<int32_t>((viewportSize.Y + extent - 1) / extent));}

::int2 ScrollComponent::ResolveViewportSize()
{
    if (this->Parent != nullptr && this->Parent->Parent != nullptr && this->Parent->Parent->get_Components() != nullptr)
    {
for (int32_t componentIndex = 0; componentIndex < this->Parent->Parent->get_Components()->get_Count(); componentIndex++) {
    ClipRectComponent* clipRectComponent = he_cpp_try_cast<ClipRectComponent>((*this->Parent->Parent->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (clipRectComponent != nullptr)
    {
return clipRectComponent->get_Size();    }
}
    }
return this->SizeValue;}

bool ScrollComponent::SetScrollOffset(int32_t scrollOffset, bool raiseEvent)
{
const int32_t clampedOffset = this->ClampOffset(static_cast<int32_t>(scrollOffset));
    if (clampedOffset == this->ScrollOffset)
    {
return false;    }
this->set_ScrollOffset(clampedOffset);
    if (raiseEvent && true)
    {
this->ScrollOffsetChanged.Invoke(this, ScrollOffset);
    }
this->ApplyContentRootOffset();
return true;}

