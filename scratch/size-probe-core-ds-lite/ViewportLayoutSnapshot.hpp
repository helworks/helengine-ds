#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;
class float3;
class LayoutComponent;
class float4;
class RoundedRectComponent;
class int2;
class TextComponent;
class SpriteComponent;
class ClipRectComponent;
class InteractableComponent;
class ScrollComponent;
class AnchorSpace;
class float2;

#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "float2.hpp"

class ViewportLayoutSnapshot
{
public:
    virtual ~ViewportLayoutSnapshot() = default;

    ::Entity* Entity;

    ::Entity* get_Entity();

    bool IsRootEntity;

    bool get_IsRootEntity();

    ::float3 LocalPosition;

    ::float3 get_LocalPosition();

    ::LayoutComponent* TrackedLayoutComponent;

    ::LayoutComponent* get_TrackedLayoutComponent();

    ::float4 AnchorDistances;

    ::float4 get_AnchorDistances();

    ::RoundedRectComponent* TrackedRoundedRectComponent;

    ::RoundedRectComponent* get_TrackedRoundedRectComponent();

    ::int2 RoundedRectSize;

    ::int2 get_RoundedRectSize();

    float RoundedRectRadius;

    float get_RoundedRectRadius();

    float RoundedRectBorderThickness;

    float get_RoundedRectBorderThickness();

    ::TextComponent* TrackedTextComponent;

    ::TextComponent* get_TrackedTextComponent();

    ::int2 TextSize;

    ::int2 get_TextSize();

    float TextFontScale;

    float get_TextFontScale();

    ::SpriteComponent* TrackedSpriteComponent;

    ::SpriteComponent* get_TrackedSpriteComponent();

    ::int2 SpriteSize;

    ::int2 get_SpriteSize();

    ::ClipRectComponent* TrackedClipRectComponent;

    ::ClipRectComponent* get_TrackedClipRectComponent();

    ::int2 ClipRectSize;

    ::int2 get_ClipRectSize();

    ::InteractableComponent* TrackedInteractableComponent;

    ::InteractableComponent* get_TrackedInteractableComponent();

    ::int2 InteractableSize;

    ::int2 get_InteractableSize();

    ::ScrollComponent* TrackedScrollComponent;

    ::ScrollComponent* get_TrackedScrollComponent();

    ::int2 ScrollSize;

    ::int2 get_ScrollSize();

    int32_t ScrollItemExtent;

    int32_t get_ScrollItemExtent();

    void Apply(::AnchorSpace* anchorSpace, ::float2 canvasOrigin, int32_t referenceWidth, int32_t referenceHeight);

    void RefreshAnchoring();

    ViewportLayoutSnapshot(::Entity* entity, bool isRootEntity);
private:
    static ::ClipRectComponent* FindClipRectComponent(::Entity* entity);

    static ::InteractableComponent* FindInteractableComponent(::Entity* entity);

    static ::LayoutComponent* FindLayoutComponent(::Entity* entity);

    static ::RoundedRectComponent* FindRoundedRectComponent(::Entity* entity);

    static ::ScrollComponent* FindScrollComponent(::Entity* entity);

    static ::SpriteComponent* FindSpriteComponent(::Entity* entity);

    static ::TextComponent* FindTextComponent(::Entity* entity);

    static float ScaleFloat(float value, double scale);

    static int32_t ScaleInt(int32_t value, double scale);

    static ::int2 ScaleInt2(::int2 value, double widthScale, double heightScale);
};
