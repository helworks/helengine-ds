#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class int2;
class Entity;
class RoundedRectComponent;
class TextComponent;
class FontAsset;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "int2.hpp"
#include "Keys.hpp"

class DebugOverlayComponent : public ::UpdateComponent
{
public:
    virtual ~DebugOverlayComponent() = default;

    bool Visible;

    bool get_Visible();
    void set_Visible(bool value);

    uint8_t RenderOrder2D;

    uint8_t get_RenderOrder2D();
    void set_RenderOrder2D(uint8_t value);

    ::int2 Padding;

    ::int2 get_Padding();
    void set_Padding(::int2 value);

    ::Keys ToggleKey;

    ::Keys get_ToggleKey();
    void set_ToggleKey(::Keys value);

    void ComponentAdded(::Entity* entity);

    DebugOverlayComponent(::FontAsset* font);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::Entity* bgEntity;

    ::Entity* textEntity;

    ::RoundedRectComponent* bg;

    ::TextComponent* text;

    ::FontAsset* font;

    bool initialized;
};
