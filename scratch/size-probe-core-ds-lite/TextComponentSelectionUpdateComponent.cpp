#ifdef DrawText
#undef DrawText
#endif
#include "TextComponentSelectionUpdateComponent.hpp"
#include "TextComponent.hpp"
#include "RuntimeTexture.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "byte4.hpp"
#include "runtime/native_string.hpp"
#include "FontAsset.hpp"
#include "TextAlignment.hpp"
#include "Entity.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_exceptions.hpp"

::TextComponent* TextComponentSelectionUpdateComponent::get_TextComponent()
{
return this->TextComponent;
}

TextComponentSelectionUpdateComponent::TextComponentSelectionUpdateComponent(::TextComponent* textComponent) : TextComponent()
{
this->TextComponent = (textComponent != nullptr ? textComponent : throw new ArgumentNullException("textComponent"));
}

void TextComponentSelectionUpdateComponent::Update()
{
this->TextComponent->UpdateSelectionInput();
}

uint8_t TextComponentSelectionUpdateComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void TextComponentSelectionUpdateComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* TextComponentSelectionUpdateComponent::get_Parent()
{
return Component::get_Parent();
}

void TextComponentSelectionUpdateComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool TextComponentSelectionUpdateComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* TextComponentSelectionUpdateComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool TextComponentSelectionUpdateComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

