#ifdef DrawText
#undef DrawText
#endif
#include "ComboBoxUpdateComponent.hpp"
#include "ComboBoxComponent.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_event.hpp"
#include "int2.hpp"
#include "FontAsset.hpp"
#include "runtime/native_list.hpp"
#include "IFocusGroup.hpp"
#include "ComboBoxItemVisual.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponent.hpp"
#include "InteractableComponent.hpp"
#include "Entity.hpp"
#include "Keys.hpp"
#include "ICamera.hpp"
#include "PointerInteraction.hpp"
#include "runtime/native_exceptions.hpp"
#include "ComboBoxUpdateComponent.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

ComboBoxUpdateComponent::ComboBoxUpdateComponent(::ComboBoxComponent* comboBox) : comboBox()
{
    if (comboBox == nullptr)
    {
throw new ArgumentNullException("comboBox");
    }
this->comboBox = comboBox;
}

void ComboBoxUpdateComponent::Update()
{
this->comboBox->Update();
}

uint8_t ComboBoxUpdateComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void ComboBoxUpdateComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* ComboBoxUpdateComponent::get_Parent()
{
return Component::get_Parent();
}

void ComboBoxUpdateComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool ComboBoxUpdateComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* ComboBoxUpdateComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool ComboBoxUpdateComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

