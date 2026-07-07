#ifdef DrawText
#undef DrawText
#endif
#include "DebugOverlayComponent.hpp"
#include "UpdateComponent.hpp"
#include "Entity.hpp"
#include "InputSystem.hpp"
#include "Core.hpp"
#include "runtime/native_list.hpp"
#include "DebugInfoRegistry.hpp"
#include "system/text/string-builder.hpp"
#include "runtime/native_string.hpp"
#include "RoundedRectComponent.hpp"
#include "int2.hpp"
#include "byte4.hpp"
#include "TextComponent.hpp"
#include "Keys.hpp"
#include "FontTightMetrics.hpp"
#include "float3.hpp"
#include "FontAsset.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "IInputBackend.hpp"
#include "InputFrameState.hpp"
#include "InputBinding.hpp"
#include "runtime/native_dictionary.hpp"
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
#include "InputGamepadButton.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
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
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "IDebugInfoProvider.hpp"
#include "runtime/native_tuple.hpp"
#include "RoundedRectCorners.hpp"
#include "RuntimeTexture.hpp"
#include "TextAlignment.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "float2.hpp"
#include "DebugOverlayComponent.hpp"
#include "system/math.hpp"
#include "system/math.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_tuple.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "system/text/string-builder.hpp"

bool DebugOverlayComponent::get_Visible()
{
return this->Visible;
}

void DebugOverlayComponent::set_Visible(bool value)
{
this->Visible = value;
}

uint8_t DebugOverlayComponent::get_RenderOrder2D()
{
return this->RenderOrder2D;
}

void DebugOverlayComponent::set_RenderOrder2D(uint8_t value)
{
this->RenderOrder2D = value;
}

::int2 DebugOverlayComponent::get_Padding()
{
return this->Padding;
}

void DebugOverlayComponent::set_Padding(::int2 value)
{
this->Padding = value;
}

::Keys DebugOverlayComponent::get_ToggleKey()
{
return this->ToggleKey;
}

void DebugOverlayComponent::set_ToggleKey(::Keys value)
{
this->ToggleKey = value;
}

void DebugOverlayComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
    if (this->initialized)
    {
return;    }
this->initialized = true;
entity->InitChildren();
this->bgEntity = new ::Entity();
this->bgEntity->set_LayerMask(entity->get_LayerMask());
this->bgEntity->InitComponents();
entity->AddChild(this->bgEntity);
this->bg = new ::RoundedRectComponent();
this->bg->set_Size(::int2(static_cast<int32_t>(200), static_cast<int32_t>(80)));
this->bg->set_Radius(6.0f);
this->bg->set_BorderThickness(1.0f);
this->bg->set_FillColor(::byte4(static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(160)));
this->bg->set_BorderColor(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(64)));
this->bg->set_RenderOrder2D(this->RenderOrder2D);
this->bgEntity->AddComponent(this->bg);
this->textEntity = new ::Entity();
this->textEntity->set_LayerMask(entity->get_LayerMask());
this->textEntity->InitComponents();
entity->AddChild(this->textEntity);
this->text = new ::TextComponent();
this->text->set_Font(this->font);
this->text->set_Color(::byte4(static_cast<uint8_t>(230), static_cast<uint8_t>(230), static_cast<uint8_t>(230), static_cast<uint8_t>(255)));
this->text->set_RenderOrder2D(static_cast<uint8_t>((this->RenderOrder2D + 1)));
this->textEntity->AddComponent(this->text);
this->bgEntity->set_Enabled(false);
this->textEntity->set_Enabled(false);
}

DebugOverlayComponent::DebugOverlayComponent(::FontAsset* font) : Visible(false), RenderOrder2D(250), Padding(::int2(static_cast<int32_t>(8), static_cast<int32_t>(6))), ToggleKey(Keys::F8), bgEntity(), textEntity(), bg(), text(), font(), initialized()
{
this->font = font;
}

void DebugOverlayComponent::Update()
{
    if (!this->initialized)
    {
return;    }
::InputSystem *inputManager = Core::Instance->Input;
const bool pressed = inputManager->WasKeyPressed(static_cast<Keys>(this->ToggleKey));
    if (pressed)
    {
this->set_Visible(!this->Visible);
    }
this->bgEntity->set_Enabled(this->Visible);
this->textEntity->set_Enabled(this->Visible);
    if (!this->Visible)
    {
return;    }
List<ValueTuple<std::string, std::string, std::string>> *rows = DebugInfoRegistry::Snapshot();
StringBuilder *sb = new StringBuilder(static_cast<int32_t>(256));
auto __localDeleteGuard_000001BF = he_cpp_make_scope_exit([&]() {
delete sb;
});
std::string current = String::Empty;
float maxW = 0.0f;
int32_t lineCount = 0;
for (int32_t i = 0; i < rows->get_Count(); i++) {
ValueTuple<std::string, std::string, std::string> row = (*rows).get_Item(static_cast<int32_t>(i));
const std::string cat = row.Item1;
const std::string key = row.Item2;
const std::string value = row.Item3;
    if (cat != current)
    {
    if (!String::IsNullOrEmpty(current))
    {
sb->Append(static_cast<char>('\n'));
    }
const std::string headerLine = String::Concat("[", cat, "]");
const std::string headerLineWithBreak = String::Concat(headerLine, "\n");
sb->Append(headerLineWithBreak);
::FontTightMetrics headerMetrics = this->font->MeasureTight(headerLine);
    if (headerMetrics.Width > maxW)
    {
maxW = headerMetrics.Width;
    }
lineCount++;
current = cat;
    }
const std::string valueLine = String::Concat(key, ": ", value);
sb->Append(valueLine);
    if (i + 1 < rows->get_Count())
    {
sb->Append(static_cast<char>('\n'));
    }
::FontTightMetrics valueMetrics = this->font->MeasureTight(valueLine);
    if (valueMetrics.Width > maxW)
    {
maxW = valueMetrics.Width;
    }
lineCount++;
}
const std::string textStr = sb->ToString();
this->text->set_Text(textStr);
    if (lineCount == 0)
    {
lineCount = 1;
    }
const int32_t w = static_cast<int32_t>(Math::Ceiling(maxW)) + this->Padding.X * 2;
const int32_t h = static_cast<int32_t>(Math::Ceiling(lineCount * this->font->LineHeight)) + this->Padding.Y * 2;
this->bg->set_Size(::int2(static_cast<int32_t>(w), static_cast<int32_t>(h)));
this->textEntity->set_Position(::float3(this->Padding.X, this->Padding.Y, 0.1f));
}

uint8_t DebugOverlayComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void DebugOverlayComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* DebugOverlayComponent::get_Parent()
{
return Component::get_Parent();
}

void DebugOverlayComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool DebugOverlayComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* DebugOverlayComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool DebugOverlayComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

