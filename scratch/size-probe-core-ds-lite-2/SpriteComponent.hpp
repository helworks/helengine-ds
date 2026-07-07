#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class ISpriteDrawable2D;
class IAnchorSizeProvider;
class IDrawable2D;
class RuntimeTexture;
class float4;
class int2;
class byte4;
class Entity;

#include "Component.hpp"
#include "ISpriteDrawable2D.hpp"
#include "IAnchorSizeProvider.hpp"
#include "runtime/native_disposable.hpp"
#include "IDrawable2D.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "byte4.hpp"

class SpriteComponent : public ::Component, public ::ISpriteDrawable2D, public ::IAnchorSizeProvider
{
public:
    virtual ~SpriteComponent() = default;

    uint8_t get_RenderOrder2D();

    void set_RenderOrder2D(uint8_t value);

    ::RuntimeTexture* Texture;

    ::RuntimeTexture* get_Texture();
    void set_Texture(::RuntimeTexture* value);

    uint8_t LayerMask;

    uint8_t get_LayerMask();
    void set_LayerMask(uint8_t value);

    ::float4 SourceRect;

    ::float4 get_SourceRect();
    void set_SourceRect(::float4 value);

    ::int2 Size;

    ::int2 get_Size();
    void set_Size(::int2 value);

    ::int2 get_AnchorSize();

    ::byte4 Color;

    ::byte4 get_Color();
    void set_Color(::byte4 value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    virtual void Draw();

    void ParentEnabledChange(bool newEnabled);

    SpriteComponent();

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    uint8_t renderOrder2D;
};
