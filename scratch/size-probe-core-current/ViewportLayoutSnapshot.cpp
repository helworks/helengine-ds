#ifdef DrawText
#undef DrawText
#endif
#include "ViewportLayoutSnapshot.hpp"
#include "system/math.hpp"
#include "LayoutComponent.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "SpriteComponent.hpp"
#include "ClipRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "ScrollComponent.hpp"
#include "int2.hpp"
#include "ViewportLayoutSnapshot.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "Entity.hpp"
#include "AnchorSpace.hpp"
#include "float2.hpp"
#include "runtime/native_event.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_nullable.hpp"
#include "RoundedRectCorners.hpp"
#include "byte4.hpp"
#include "RuntimeTexture.hpp"
#include "FontAsset.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "PointerCursorKind.hpp"
#include "PointerInteraction.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "system/math.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_nullable.hpp"

::Entity* ViewportLayoutSnapshot::get_Entity()
{
return this->Entity;
}

bool ViewportLayoutSnapshot::get_IsRootEntity()
{
return this->IsRootEntity;
}

::float3 ViewportLayoutSnapshot::get_LocalPosition()
{
return this->LocalPosition;
}

::LayoutComponent* ViewportLayoutSnapshot::get_TrackedLayoutComponent()
{
return this->TrackedLayoutComponent;
}

::float4 ViewportLayoutSnapshot::get_AnchorDistances()
{
return this->AnchorDistances;
}

::RoundedRectComponent* ViewportLayoutSnapshot::get_TrackedRoundedRectComponent()
{
return this->TrackedRoundedRectComponent;
}

::int2 ViewportLayoutSnapshot::get_RoundedRectSize()
{
return this->RoundedRectSize;
}

float ViewportLayoutSnapshot::get_RoundedRectRadius()
{
return this->RoundedRectRadius;
}

float ViewportLayoutSnapshot::get_RoundedRectBorderThickness()
{
return this->RoundedRectBorderThickness;
}

::TextComponent* ViewportLayoutSnapshot::get_TrackedTextComponent()
{
return this->TrackedTextComponent;
}

::int2 ViewportLayoutSnapshot::get_TextSize()
{
return this->TextSize;
}

float ViewportLayoutSnapshot::get_TextFontScale()
{
return this->TextFontScale;
}

::SpriteComponent* ViewportLayoutSnapshot::get_TrackedSpriteComponent()
{
return this->TrackedSpriteComponent;
}

::int2 ViewportLayoutSnapshot::get_SpriteSize()
{
return this->SpriteSize;
}

::ClipRectComponent* ViewportLayoutSnapshot::get_TrackedClipRectComponent()
{
return this->TrackedClipRectComponent;
}

::int2 ViewportLayoutSnapshot::get_ClipRectSize()
{
return this->ClipRectSize;
}

::InteractableComponent* ViewportLayoutSnapshot::get_TrackedInteractableComponent()
{
return this->TrackedInteractableComponent;
}

::int2 ViewportLayoutSnapshot::get_InteractableSize()
{
return this->InteractableSize;
}

::ScrollComponent* ViewportLayoutSnapshot::get_TrackedScrollComponent()
{
return this->TrackedScrollComponent;
}

::int2 ViewportLayoutSnapshot::get_ScrollSize()
{
return this->ScrollSize;
}

int32_t ViewportLayoutSnapshot::get_ScrollItemExtent()
{
return this->ScrollItemExtent;
}

void ViewportLayoutSnapshot::Apply(::AnchorSpace* anchorSpace, ::float2 canvasOrigin, int32_t referenceWidth, int32_t referenceHeight)
{
const double widthScale = referenceWidth > 0 ? static_cast<double>(anchorSpace->Size.X) / referenceWidth : 1.0;
const double heightScale = referenceHeight > 0 ? static_cast<double>(anchorSpace->Size.Y) / referenceHeight : 1.0;
const double scalarScale = Math::Min(widthScale, heightScale);
float positionX = ViewportLayoutSnapshot::ScaleFloat(this->LocalPosition.X, widthScale);
float positionY = ViewportLayoutSnapshot::ScaleFloat(this->LocalPosition.Y, heightScale);
    if (this->IsRootEntity)
    {
positionX += canvasOrigin.X;
positionY += canvasOrigin.Y;
    }
this->Entity->set_LocalPosition(::float3(positionX, positionY, this->LocalPosition.Z));
    if (this->TrackedLayoutComponent != nullptr)
    {
this->TrackedLayoutComponent->set_AnchorDistances(([&]() {
auto __ctor_arg_00000189 = ViewportLayoutSnapshot::ScaleFloat(this->AnchorDistances.X, widthScale);
auto __ctor_arg_0000018A = ViewportLayoutSnapshot::ScaleFloat(this->AnchorDistances.Y, widthScale);
auto __ctor_arg_0000018B = ViewportLayoutSnapshot::ScaleFloat(this->AnchorDistances.Z, heightScale);
auto __ctor_arg_0000018C = ViewportLayoutSnapshot::ScaleFloat(this->AnchorDistances.W, heightScale);
return ::float4(__ctor_arg_00000189, __ctor_arg_0000018A, __ctor_arg_0000018B, __ctor_arg_0000018C);
})());
    }
    if (this->TrackedRoundedRectComponent != nullptr)
    {
this->TrackedRoundedRectComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->RoundedRectSize, widthScale, heightScale));
this->TrackedRoundedRectComponent->set_Radius(ViewportLayoutSnapshot::ScaleFloat(this->RoundedRectRadius, scalarScale));
this->TrackedRoundedRectComponent->set_BorderThickness(ViewportLayoutSnapshot::ScaleFloat(this->RoundedRectBorderThickness, scalarScale));
    }
    if (this->TrackedTextComponent != nullptr)
    {
this->TrackedTextComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->TextSize, widthScale, heightScale));
this->TrackedTextComponent->set_FontScale(ViewportLayoutSnapshot::ScaleFloat(this->TextFontScale, scalarScale));
    }
    if (this->TrackedSpriteComponent != nullptr)
    {
this->TrackedSpriteComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->SpriteSize, widthScale, heightScale));
    }
    if (this->TrackedClipRectComponent != nullptr)
    {
this->TrackedClipRectComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->ClipRectSize, widthScale, heightScale));
    }
    if (this->TrackedInteractableComponent != nullptr)
    {
this->TrackedInteractableComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->InteractableSize, widthScale, heightScale));
    }
    if (this->TrackedScrollComponent != nullptr)
    {
this->TrackedScrollComponent->set_Size(ViewportLayoutSnapshot::ScaleInt2(this->ScrollSize, widthScale, heightScale));
this->TrackedScrollComponent->set_ItemExtent(ViewportLayoutSnapshot::ScaleInt(static_cast<int32_t>(this->ScrollItemExtent), scalarScale));
    }
}

void ViewportLayoutSnapshot::RefreshAnchoring()
{
    if (this->TrackedLayoutComponent != nullptr)
    {
this->TrackedLayoutComponent->RefreshAnchoring();
    }
}

ViewportLayoutSnapshot::ViewportLayoutSnapshot(::Entity* entity, bool isRootEntity) : Entity(), IsRootEntity(), LocalPosition(), TrackedLayoutComponent(), AnchorDistances(), TrackedRoundedRectComponent(), RoundedRectSize(), RoundedRectRadius(), RoundedRectBorderThickness(), TrackedTextComponent(), TextSize(), TextFontScale(), TrackedSpriteComponent(), SpriteSize(), TrackedClipRectComponent(), ClipRectSize(), TrackedInteractableComponent(), InteractableSize(), TrackedScrollComponent(), ScrollSize(), ScrollItemExtent(0)
{
this->Entity = (entity != nullptr ? entity : throw new ArgumentNullException("entity"));
this->IsRootEntity = isRootEntity;
this->LocalPosition = entity->get_LocalPosition();
this->TrackedLayoutComponent = ViewportLayoutSnapshot::FindLayoutComponent(entity);
    if (this->TrackedLayoutComponent != nullptr)
    {
this->AnchorDistances = this->TrackedLayoutComponent->AnchorDistances;
    }
this->TrackedRoundedRectComponent = ViewportLayoutSnapshot::FindRoundedRectComponent(entity);
    if (this->TrackedRoundedRectComponent != nullptr)
    {
this->RoundedRectSize = this->TrackedRoundedRectComponent->Size;
this->RoundedRectRadius = this->TrackedRoundedRectComponent->Radius;
this->RoundedRectBorderThickness = this->TrackedRoundedRectComponent->BorderThickness;
    }
this->TrackedTextComponent = ViewportLayoutSnapshot::FindTextComponent(entity);
    if (this->TrackedTextComponent != nullptr)
    {
this->TextSize = this->TrackedTextComponent->get_Size();
this->TextFontScale = this->TrackedTextComponent->get_FontScale();
    }
this->TrackedSpriteComponent = ViewportLayoutSnapshot::FindSpriteComponent(entity);
    if (this->TrackedSpriteComponent != nullptr)
    {
this->SpriteSize = this->TrackedSpriteComponent->Size;
    }
this->TrackedClipRectComponent = ViewportLayoutSnapshot::FindClipRectComponent(entity);
    if (this->TrackedClipRectComponent != nullptr)
    {
this->ClipRectSize = this->TrackedClipRectComponent->get_Size();
    }
this->TrackedInteractableComponent = ViewportLayoutSnapshot::FindInteractableComponent(entity);
    if (this->TrackedInteractableComponent != nullptr)
    {
this->InteractableSize = this->TrackedInteractableComponent->Size;
    }
this->TrackedScrollComponent = ViewportLayoutSnapshot::FindScrollComponent(entity);
    if (this->TrackedScrollComponent != nullptr)
    {
this->ScrollSize = this->TrackedScrollComponent->get_Size();
this->ScrollItemExtent = this->TrackedScrollComponent->get_ItemExtent();
    }
}

::ClipRectComponent* ViewportLayoutSnapshot::FindClipRectComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    ClipRectComponent* component = he_cpp_try_cast<ClipRectComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::InteractableComponent* ViewportLayoutSnapshot::FindInteractableComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    InteractableComponent* component = he_cpp_try_cast<InteractableComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::LayoutComponent* ViewportLayoutSnapshot::FindLayoutComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    LayoutComponent* component = he_cpp_try_cast<LayoutComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::RoundedRectComponent* ViewportLayoutSnapshot::FindRoundedRectComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    RoundedRectComponent* component = he_cpp_try_cast<RoundedRectComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::ScrollComponent* ViewportLayoutSnapshot::FindScrollComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    ScrollComponent* component = he_cpp_try_cast<ScrollComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::SpriteComponent* ViewportLayoutSnapshot::FindSpriteComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    SpriteComponent* component = he_cpp_try_cast<SpriteComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

::TextComponent* ViewportLayoutSnapshot::FindTextComponent(::Entity* entity)
{
    if (entity->get_Components() == nullptr)
    {
return nullptr;    }
for (int32_t componentIndex = 0; componentIndex < entity->get_Components()->get_Count(); componentIndex++) {
    TextComponent* component = he_cpp_try_cast<TextComponent>((*entity->get_Components()).get_Item(static_cast<int32_t>(componentIndex)));
    if (component != nullptr)
    {
return component;    }
}
return nullptr;}

float ViewportLayoutSnapshot::ScaleFloat(float value, double scale)
{
return static_cast<float>((value * scale));}

int32_t ViewportLayoutSnapshot::ScaleInt(int32_t value, double scale)
{
    if (value == 0)
    {
return 0;    }
return Math::Max(static_cast<int32_t>(1), static_cast<int32_t>(static_cast<int32_t>(Math::Round(value * scale))));}

::int2 ViewportLayoutSnapshot::ScaleInt2(::int2 value, double widthScale, double heightScale)
{
return ([&]() {
auto __ctor_arg_0000018D = static_cast<int32_t>(ViewportLayoutSnapshot::ScaleInt(static_cast<int32_t>(value.X), widthScale));
auto __ctor_arg_0000018E = static_cast<int32_t>(ViewportLayoutSnapshot::ScaleInt(static_cast<int32_t>(value.Y), heightScale));
return ::int2(__ctor_arg_0000018D, __ctor_arg_0000018E);
})();}

