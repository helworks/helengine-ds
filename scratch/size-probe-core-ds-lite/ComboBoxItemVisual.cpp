#ifdef DrawText
#undef DrawText
#endif
#include "ComboBoxItemVisual.hpp"
#include "Entity.hpp"
#include "RoundedRectComponent.hpp"
#include "InteractableComponent.hpp"
#include "TextComponent.hpp"
#include "runtime/native_event.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "PointerInteraction.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "RoundedRectCorners.hpp"
#include "byte4.hpp"
#include "PointerCursorKind.hpp"
#include "RuntimeTexture.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/action.hpp"
#include "ComboBoxItemVisual.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"

::Entity* ComboBoxItemVisual::get_Root()
{
return this->Root;
}

::RoundedRectComponent* ComboBoxItemVisual::get_Background()
{
return this->Background;
}

::Entity* ComboBoxItemVisual::get_LabelHost()
{
return this->LabelHost;
}

::TextComponent* ComboBoxItemVisual::get_Label()
{
return this->Label;
}

::InteractableComponent* ComboBoxItemVisual::get_Interactable()
{
return this->Interactable;
}

int32_t ComboBoxItemVisual::get_Index()
{
return this->Index;
}

void ComboBoxItemVisual::set_Index(int32_t value)
{
this->Index = value;
}

bool ComboBoxItemVisual::get_IsHovering()
{
return this->IsHovering;
}

void ComboBoxItemVisual::set_IsHovering(bool value)
{
this->IsHovering = value;
}

bool ComboBoxItemVisual::get_IsPressed()
{
return this->IsPressed;
}

void ComboBoxItemVisual::set_IsPressed(bool value)
{
this->IsPressed = value;
}

ComboBoxItemVisual::ComboBoxItemVisual(::FontAsset* font, uint16_t layerMask, uint8_t backgroundOrder, uint8_t textOrder) : Root(), Background(), LabelHost(), Label(), Interactable(), CursorEvent(), Index(0), IsHovering(), IsPressed()
{
    if (font == nullptr)
    {
throw new ArgumentNullException("font");
    }
this->Root = new ::Entity();
this->Root->set_LayerMask(layerMask);
this->Root->set_Enabled(true);
this->Root->InitComponents();
this->Root->InitChildren();
this->Background = new ::RoundedRectComponent();
this->Background->set_RenderOrder2D(backgroundOrder);
this->Background->set_BorderThickness(1.0f);
this->Root->AddComponent(this->Background);
this->Interactable = new ::InteractableComponent();
this->Interactable->CursorEvent += Event::Bind(this, static_cast<void (ComboBoxItemVisual::*)(int2, int2, PointerInteraction)>(&ComboBoxItemVisual::HandleCursorEvent));
this->Root->AddComponent(this->Interactable);
this->LabelHost = new ::Entity();
this->LabelHost->set_LayerMask(layerMask);
this->LabelHost->set_Enabled(true);
this->LabelHost->InitComponents();
this->Root->AddChild(this->LabelHost);
this->Label = new ::TextComponent();
this->Label->set_Font(font);
this->Label->set_RenderOrder2D(textOrder);
this->LabelHost->AddComponent(this->Label);
}

void ComboBoxItemVisual::HandleCursorEvent(::int2 relPos, ::int2 delta, ::PointerInteraction state)
{
    if (true)
    {
this->CursorEvent.Invoke(this, relPos, delta, state);
    }
}

