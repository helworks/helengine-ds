#ifdef DrawText
#undef DrawText
#endif
#include "DebugComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "UpdateComponent.hpp"
#include "NativeOwnership.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "DebugComponent.hpp"
#include "runtime/native_list.hpp"
#include "Entity.hpp"
#include "TextComponent.hpp"
#include "Core.hpp"
#include "RuntimeMemoryCounters.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "system/math.hpp"
#include "int2.hpp"
#include "float3.hpp"
#include "byte4.hpp"
#include "FontAsset.hpp"
#include "runtime/array.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "RuntimeTexture.hpp"
#include "TextAlignment.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
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
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeMemoryDiagnosticsSnapshot.hpp"
#include "IRuntimeDiagnosticsProvider.hpp"
#include "float2.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/number.hpp"
#include "system/diagnostics/stopwatch.hpp"

double DebugComponent::get_RefreshIntervalSeconds()
{
return this->RefreshIntervalSecondsValue;}

void DebugComponent::set_RefreshIntervalSeconds(double value)
{
    if (value < 0.0)
    {
throw ([&]() {
auto __ctor_arg_000001B7 = "value";
auto __ctor_arg_000001B8 = "Refresh interval must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B7, __ctor_arg_000001B8);
})();
    }
this->RefreshIntervalSecondsValue = value;
}

::int2 DebugComponent::get_Padding()
{
return this->PaddingValue;}

void DebugComponent::set_Padding(::int2 value)
{
this->PaddingValue = value;
this->ApplyPadding();
}

uint8_t DebugComponent::get_RenderOrder2D()
{
return this->RenderOrder2DValue;}

void DebugComponent::set_RenderOrder2D(uint8_t value)
{
    if (this->RenderOrder2DValue == value)
    {
return;    }
this->RenderOrder2DValue = value;
this->ApplyRenderOrder();
}

::FontAsset* DebugComponent::get_Font()
{
return this->FontValue;}

void DebugComponent::set_Font(::FontAsset* value)
{
    if ((this->FontValue == value))
    {
return;    }
this->FontValue = value;
this->RefreshOverlayActivation();
}

float DebugComponent::get_FontScale()
{
return this->FontScaleValue;}

void DebugComponent::set_FontScale(float value)
{
    if (value <= 0.0f)
    {
throw ([&]() {
auto __ctor_arg_000001B9 = "value";
auto __ctor_arg_000001BA = "Font scale must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_000001B9, __ctor_arg_000001BA);
})();
    }
    if (this->FontScaleValue == value)
    {
return;    }
this->FontScaleValue = value;
this->RefreshOverlayActivation();
}

const std::string& DebugComponent::get_UpdateFpsText()
{
return this->UpdateFpsText;
}

void DebugComponent::set_UpdateFpsText(std::string value)
{
this->UpdateFpsText = value;
}

const std::string& DebugComponent::get_RenderFpsText()
{
return this->RenderFpsText;
}

void DebugComponent::set_RenderFpsText(std::string value)
{
this->RenderFpsText = value;
}

const std::string& DebugComponent::get_ResidentMemoryText()
{
return this->ResidentMemoryText;
}

void DebugComponent::set_ResidentMemoryText(std::string value)
{
this->ResidentMemoryText = value;
}

const std::string& DebugComponent::get_CommittedMemoryText()
{
return this->CommittedMemoryText;
}

void DebugComponent::set_CommittedMemoryText(std::string value)
{
this->CommittedMemoryText = value;
}

const std::string& DebugComponent::get_Drawables2DText()
{
return this->Drawables2DText;
}

void DebugComponent::set_Drawables2DText(std::string value)
{
this->Drawables2DText = value;
}

const std::string& DebugComponent::get_Drawables3DText()
{
return this->Drawables3DText;
}

void DebugComponent::set_Drawables3DText(std::string value)
{
this->Drawables3DText = value;
}

void DebugComponent::ClearAdditionalLine(std::string id)
{
    if (String::IsNullOrWhiteSpace(id))
    {
throw ([&]() {
auto __ctor_arg_000001BB = "Additional debug line id must be provided.";
auto __ctor_arg_000001BC = "id";
return new ArgumentException(__ctor_arg_000001BB, __ctor_arg_000001BC);
})();
    }
    if (!AdditionalLinesById->Remove(id))
    {
return;    }
AdditionalLineIds->Remove(id);
DebugComponent::RefreshAdditionalLinesOnActiveComponents();
}

void DebugComponent::ClearAdditionalLines()
{
    if (AdditionalLineIds->get_Count() == 0 && AdditionalLinesById->get_Count() == 0)
    {
return;    }
AdditionalLineIds->Clear();
AdditionalLinesById->Clear();
DebugComponent::RefreshAdditionalLinesOnActiveComponents();
}

void DebugComponent::ComponentAdded(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
UpdateComponent::ComponentAdded(entity);
this->RefreshOverlayActivation();
}

void DebugComponent::ComponentRemoved(::Entity* entity)
{
this->TearDownOverlay();
delete this->MemoryCountersValue;
this->MemoryCountersValue = nullptr;
UpdateComponent::ComponentRemoved(entity);
}

DebugComponent::DebugComponent() : UpdateFpsText(), RenderFpsText(), ResidentMemoryText(), CommittedMemoryText(), Drawables2DText(), Drawables3DText(), FontValue(), FontScaleValue(1.0f), OverlayHost(), UpdateFpsRowHost(), RenderFpsRowHost(), ResidentMemoryRowHost(), CommittedMemoryRowHost(), Drawables2DRowHost(), Drawables3DRowHost(), AdditionalLineRowHosts(), UpdateFpsTextComponent(), RenderFpsTextComponent(), ResidentMemoryTextComponent(), CommittedMemoryTextComponent(), Drawables2DTextComponent(), Drawables3DTextComponent(), AdditionalLineTextComponents(), LastSampleElapsedSeconds(0), UpdateFrameCount(0), RenderFrameCount(0), Initialized(), MemoryCountersValue(), RefreshIntervalSecondsValue(0.5), PaddingValue(::int2(static_cast<int32_t>(8), static_cast<int32_t>(6))), RenderOrder2DValue(250)
{
this->MemoryCountersValue = new ::RuntimeMemoryCounters();
this->AdditionalLineRowHosts = new List<::Entity*>();
this->AdditionalLineTextComponents = new List<::TextComponent*>();
this->ResetSamplingWindow();
}

void DebugComponent::ParentEnabledChange(bool newEnabled)
{
UpdateComponent::ParentEnabledChange(newEnabled);
this->ResetSamplingWindow();
this->ApplyVisibleText();
}

void DebugComponent::RecordRenderFrame()
{
for (int32_t i = ActiveComponents->get_Count() - 1; i >= 0; i--) {
::DebugComponent *component = (*ActiveComponents).get_Item(static_cast<int32_t>(i));
    if (component->Initialized && component->Parent != nullptr && component->Parent->get_IsHierarchyEnabled())
    {
component->RenderFrameCount++;
    }
}
}

void DebugComponent::RecordUpdateFrame()
{
for (int32_t i = ActiveComponents->get_Count() - 1; i >= 0; i--) {
::DebugComponent *component = (*ActiveComponents).get_Item(static_cast<int32_t>(i));
    if (component->Initialized && component->Parent != nullptr && component->Parent->get_IsHierarchyEnabled())
    {
component->UpdateFrameCount++;
    }
}
}

void DebugComponent::SetAdditionalLine(std::string id, std::string text)
{
    if (String::IsNullOrWhiteSpace(id))
    {
throw ([&]() {
auto __ctor_arg_000001BD = "Additional debug line id must be provided.";
auto __ctor_arg_000001BE = "id";
return new ArgumentException(__ctor_arg_000001BD, __ctor_arg_000001BE);
})();
    }
else {
    if (String::IsNullOrEmpty(text))
    {
throw new ArgumentNullException("text");
    }
}
std::string existingText;
    if (AdditionalLinesById->TryGetValue(id, existingText))
    {
    if (existingText == text)
    {
return;    }
    }
else {
AdditionalLineIds->Add(id);
}
(*AdditionalLinesById).get_Item(id) = text;
DebugComponent::RefreshAdditionalLinesOnActiveComponents();
}

void DebugComponent::Update()
{
    if (!this->Initialized)
    {
return;    }
this->TryRefreshOverlay();
}

uint8_t DebugComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void DebugComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* DebugComponent::get_Parent()
{
return Component::get_Parent();
}

void DebugComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool DebugComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* DebugComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool DebugComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

const double DebugComponent::BytesPerMegabyte = 1024.0 * 1024.0;

List<::DebugComponent*>* DebugComponent::ActiveComponents = new List<::DebugComponent*>();

List<std::string>* DebugComponent::AdditionalLineIds = new List<std::string>();

Dictionary<std::string, std::string>* DebugComponent::AdditionalLinesById = new Dictionary<std::string, std::string>();

void DebugComponent::ApplyAdditionalLineText()
{
    if (this->AdditionalLineTextComponents->get_Count() != AdditionalLineIds->get_Count())
    {
throw new InvalidOperationException("Additional debug line rows must be synchronized before text can be applied.");
    }
for (int32_t index = 0; index < AdditionalLineIds->get_Count(); index++) {
const std::string id = (*AdditionalLineIds).get_Item(static_cast<int32_t>(index));
std::string text;
    if (!AdditionalLinesById->TryGetValue(id, text))
    {
throw new InvalidOperationException("Additional debug line text was missing for a registered id.");
    }
(*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(index))->set_Text(text);
}
}

void DebugComponent::ApplyFont()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->UpdateFpsTextComponent != nullptr)
    {
this->UpdateFpsTextComponent->set_Font(this->get_Font());
this->UpdateFpsTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->RenderFpsTextComponent != nullptr)
    {
this->RenderFpsTextComponent->set_Font(this->get_Font());
this->RenderFpsTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->ResidentMemoryTextComponent != nullptr)
    {
this->ResidentMemoryTextComponent->set_Font(this->get_Font());
this->ResidentMemoryTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->CommittedMemoryTextComponent != nullptr)
    {
this->CommittedMemoryTextComponent->set_Font(this->get_Font());
this->CommittedMemoryTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->Drawables2DTextComponent != nullptr)
    {
this->Drawables2DTextComponent->set_Font(this->get_Font());
this->Drawables2DTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->Drawables3DTextComponent != nullptr)
    {
this->Drawables3DTextComponent->set_Font(this->get_Font());
this->Drawables3DTextComponent->set_FontScale(this->get_FontScale());
    }
for (int32_t index = 0; index < this->AdditionalLineTextComponents->get_Count(); index++) {
(*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(index))->set_Font(this->get_Font());
(*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(index))->set_FontScale(this->get_FontScale());
}
    if (this->RenderFpsRowHost != nullptr)
    {
this->RenderFpsRowHost->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale(), 0.1f));
    }
    if (this->ResidentMemoryRowHost != nullptr)
    {
this->ResidentMemoryRowHost->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale() * 2.0f, 0.2f));
    }
    if (this->CommittedMemoryRowHost != nullptr)
    {
this->CommittedMemoryRowHost->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale() * 3.0f, 0.3f));
    }
    if (this->Drawables2DRowHost != nullptr)
    {
this->Drawables2DRowHost->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale() * 4.0f, 0.4f));
    }
    if (this->Drawables3DRowHost != nullptr)
    {
this->Drawables3DRowHost->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale() * 5.0f, 0.5f));
    }
for (int32_t index = 0; index < this->AdditionalLineRowHosts->get_Count(); index++) {
const float rowIndex = 6.0f + index;
(*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(index))->set_LocalPosition(::float3(0.0f, this->get_Font()->LineHeight * this->get_FontScale() * rowIndex, 0.5f + (0.1f * index)));
}
}

void DebugComponent::ApplyPadding()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->OverlayHost == nullptr)
    {
return;    }
this->OverlayHost->set_LocalPosition(::float3(this->get_Padding().X, this->get_Padding().Y, 0.0f));
}

void DebugComponent::ApplyRenderOrder()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->UpdateFpsTextComponent != nullptr)
    {
this->UpdateFpsTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->RenderFpsTextComponent != nullptr)
    {
this->RenderFpsTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->ResidentMemoryTextComponent != nullptr)
    {
this->ResidentMemoryTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->CommittedMemoryTextComponent != nullptr)
    {
this->CommittedMemoryTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->Drawables2DTextComponent != nullptr)
    {
this->Drawables2DTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->Drawables3DTextComponent != nullptr)
    {
this->Drawables3DTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
for (int32_t index = 0; index < this->AdditionalLineTextComponents->get_Count(); index++) {
(*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(index))->set_RenderOrder2D(this->get_RenderOrder2D());
}
}

void DebugComponent::ApplyVisibleText()
{
this->SyncAdditionalLineRows();
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->UpdateFpsTextComponent != nullptr)
    {
this->UpdateFpsTextComponent->set_Text(this->UpdateFpsText);
    }
    if (this->RenderFpsTextComponent != nullptr)
    {
this->RenderFpsTextComponent->set_Text(this->RenderFpsText);
    }
    if (this->ResidentMemoryTextComponent != nullptr)
    {
this->ResidentMemoryTextComponent->set_Text(this->ResidentMemoryText);
    }
    if (this->CommittedMemoryTextComponent != nullptr)
    {
this->CommittedMemoryTextComponent->set_Text(this->CommittedMemoryText);
    }
    if (this->Drawables2DTextComponent != nullptr)
    {
this->Drawables2DTextComponent->set_Text(this->Drawables2DText);
    }
    if (this->Drawables3DTextComponent != nullptr)
    {
this->Drawables3DTextComponent->set_Text(this->Drawables3DText);
    }
this->ApplyAdditionalLineText();
}

bool DebugComponent::AreAdditionalLineRowsLive()
{
    if (this->AdditionalLineRowHosts->get_Count() != AdditionalLineIds->get_Count() || this->AdditionalLineTextComponents->get_Count() != AdditionalLineIds->get_Count())
    {
return false;    }
for (int32_t index = 0; index < AdditionalLineIds->get_Count(); index++) {
    if (!this->IsLiveRow((*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(index)), this->OverlayHost, (*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(index))))
    {
return false;    }
}
return true;}

void DebugComponent::BuildOverlay()
{
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("DebugComponent must be attached before its overlay can be created.");
    }
    if (this->get_Font() == nullptr)
    {
throw new InvalidOperationException("DebugComponent overlay creation requires a font.");
    }
    if (this->Initialized)
    {
return;    }
    if (this->Parent->get_Children() == nullptr)
    {
this->Parent->InitChildren();
    }
this->OverlayHost = new ::Entity();
this->OverlayHost->set_LayerMask(this->Parent->get_LayerMask());
this->OverlayHost->InitChildren();
this->OverlayHost->InitComponents();
this->Parent->AddChild(this->OverlayHost);
this->UpdateFpsRowHost = this->CreateRowHost();
this->UpdateFpsTextComponent = this->CreateRowTextComponent(this->UpdateFpsRowHost);
this->RenderFpsRowHost = this->CreateRowHost();
this->RenderFpsTextComponent = this->CreateRowTextComponent(this->RenderFpsRowHost);
this->ResidentMemoryRowHost = this->CreateRowHost();
this->ResidentMemoryTextComponent = this->CreateRowTextComponent(this->ResidentMemoryRowHost);
this->CommittedMemoryRowHost = this->CreateRowHost();
this->CommittedMemoryTextComponent = this->CreateRowTextComponent(this->CommittedMemoryRowHost);
this->Drawables2DRowHost = this->CreateRowHost();
this->Drawables2DTextComponent = this->CreateRowTextComponent(this->Drawables2DRowHost);
this->Drawables3DRowHost = this->CreateRowHost();
this->Drawables3DTextComponent = this->CreateRowTextComponent(this->Drawables3DRowHost);
this->Initialized = true;
ActiveComponents->Add(this);
this->SyncAdditionalLineRows();
this->ApplyFont();
}

::Entity* DebugComponent::CreateRowHost()
{
::Entity *rowHost = new ::Entity();
rowHost->set_LayerMask(this->Parent->get_LayerMask());
rowHost->InitChildren();
rowHost->InitComponents();
this->OverlayHost->AddChild(rowHost);
return rowHost;}

::TextComponent* DebugComponent::CreateRowTextComponent(::Entity* rowHost)
{
    if (rowHost == nullptr)
    {
throw new ArgumentNullException("rowHost");
    }
::TextComponent *textComponent = new ::TextComponent();
textComponent->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
textComponent->set_Font(this->get_Font());
textComponent->set_FontScale(this->get_FontScale());
rowHost->AddComponent(textComponent);
return textComponent;}

bool DebugComponent::EnsureOverlayHierarchyIsLive()
{
    if (!this->Initialized)
    {
return false;    }
return this->IsBaseOverlayHierarchyLive() && this->AreAdditionalLineRowsLive();}

std::string DebugComponent::FormatMegabytes(uint64_t bytes)
{
const double megabytes = bytes / BytesPerMegabyte;
return String::Concat(this->FormatOneDecimal(megabytes), " MB");}

std::string DebugComponent::FormatOneDecimal(double value)
{
    if (Number::IsNaN(value) || Number::IsInfinity(value) || value > 2147483647 / 10.0 || value < -2147483648 / 10.0)
    {
return "--";    }
const int32_t tenths = static_cast<int32_t>(Math::Round(value * 10.0, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero)));
const int32_t whole = tenths / 10;
const int32_t fractional = Math::Abs(static_cast<int32_t>(tenths % 10));
return String::Concat(std::to_string(whole), ".", std::to_string(fractional));}

std::string DebugComponent::FormatPerformanceOverlayPrimaryLine(::Core* core)
{
return String::Concat("P1 Txt", this->FormatOneDecimal(core->PerformanceOverlayTriangleSetupMilliseconds), " H", DebugComponent::FormatRoundedMetric(core->PerformanceOverlayTrianglePrepMilliseconds), " M", std::to_string(core->PerformanceOverlaySubmittedTriangleCount), " F", std::to_string(core->PerformanceOverlayDispatchCount), " G", DebugComponent::FormatRoundedMetric(core->PerformanceOverlayTriangleEmitMilliseconds));}

std::string DebugComponent::FormatPerformanceOverlaySecondaryLine(::Core* core)
{
return String::Concat("P2 Geo", this->FormatOneDecimal(core->PerformanceOverlayPacketEncodeMilliseconds), " Fl", this->FormatOneDecimal(core->PerformanceOverlaySubmitMilliseconds), " Pr", this->FormatOneDecimal(core->PerformanceOverlayWaitMilliseconds));}

std::string DebugComponent::FormatRenderFpsText(double renderFps, double drawMilliseconds)
{
return String::Concat("Render FPS: ", this->FormatOneDecimal(renderFps), " (", this->FormatOneDecimal(drawMilliseconds), " ms)");}

std::string DebugComponent::FormatRoundedMetric(double value)
{
    if (Number::IsNaN(value) || Number::IsInfinity(value) || value > 2147483647 || value < -2147483648)
    {
return "--";    }
return std::to_string((static_cast<int32_t>(Math::Round(value, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero)))));}

std::string DebugComponent::FormatUpdateFpsText(double updateFps, double updateMilliseconds)
{
return String::Concat("Update FPS: ", this->FormatOneDecimal(updateFps), " (", this->FormatOneDecimal(updateMilliseconds), " ms)");}

bool DebugComponent::IsBaseOverlayHierarchyLive()
{
    if (!this->Initialized)
    {
return false;    }
return this->OverlayHost != nullptr && !this->OverlayHost->get_IsDisposed() && this->OverlayHost->get_ParentUnsafe() == this->Parent && this->IsLiveRow(this->UpdateFpsRowHost, this->OverlayHost, this->UpdateFpsTextComponent) && this->IsLiveRow(this->RenderFpsRowHost, this->OverlayHost, this->RenderFpsTextComponent) && this->IsLiveRow(this->ResidentMemoryRowHost, this->OverlayHost, this->ResidentMemoryTextComponent) && this->IsLiveRow(this->CommittedMemoryRowHost, this->OverlayHost, this->CommittedMemoryTextComponent) && this->IsLiveRow(this->Drawables2DRowHost, this->OverlayHost, this->Drawables2DTextComponent) && this->IsLiveRow(this->Drawables3DRowHost, this->OverlayHost, this->Drawables3DTextComponent);}

bool DebugComponent::IsLiveRow(::Entity* rowHost, ::Entity* overlayHost, ::TextComponent* textComponent)
{
return rowHost != nullptr && !rowHost->get_IsDisposed() && rowHost->get_ParentUnsafe() == overlayHost && textComponent != nullptr && !textComponent->get_IsDisposed() && textComponent->get_ParentUnsafe() == rowHost;}

void DebugComponent::RefreshAdditionalLinesOnActiveComponents()
{
for (int32_t i = ActiveComponents->get_Count() - 1; i >= 0; i--) {
::DebugComponent *component = (*ActiveComponents).get_Item(static_cast<int32_t>(i));
    if (component->Initialized && component->Parent != nullptr && component->Parent->get_IsHierarchyEnabled())
    {
    if (!component->IsBaseOverlayHierarchyLive())
    {
component->ReleaseOverlayReferences();
continue;
    }
component->SyncAdditionalLineRows();
component->ApplyFont();
component->ApplyRenderOrder();
component->ApplyVisibleText();
    }
}
}

void DebugComponent::RefreshOverlayActivation()
{
    if (this->Parent == nullptr)
    {
return;    }
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
    }
    if (this->get_Font() == nullptr)
    {
this->TearDownOverlay();
return;    }
    if (!this->Initialized)
    {
this->BuildOverlay();
this->ResetSamplingWindow();
this->ApplyVisibleText();
this->ApplyPadding();
this->ApplyRenderOrder();
return;    }
this->ApplyFont();
this->ApplyPadding();
this->ApplyRenderOrder();
this->ApplyVisibleText();
}

void DebugComponent::ReleaseOverlayReferences()
{
ActiveComponents->Remove(this);
this->OverlayHost = nullptr;
this->UpdateFpsRowHost = nullptr;
this->RenderFpsRowHost = nullptr;
this->ResidentMemoryRowHost = nullptr;
this->CommittedMemoryRowHost = nullptr;
this->Drawables2DRowHost = nullptr;
this->Drawables3DRowHost = nullptr;
this->UpdateFpsTextComponent = nullptr;
this->RenderFpsTextComponent = nullptr;
this->ResidentMemoryTextComponent = nullptr;
this->CommittedMemoryTextComponent = nullptr;
this->Drawables2DTextComponent = nullptr;
this->Drawables3DTextComponent = nullptr;
this->AdditionalLineRowHosts->Clear();
this->AdditionalLineTextComponents->Clear();
this->Initialized = false;
}

void DebugComponent::ResetSamplingWindow()
{
this->UpdateFrameCount = 0;
this->RenderFrameCount = 0;
::Core *core = Core::Instance;
this->LastSampleElapsedSeconds = core == nullptr ? 0.0 : core->TotalElapsedSeconds;
this->set_UpdateFpsText("Update FPS: -- (-- ms)");
this->set_RenderFpsText("Render FPS: -- (-- ms)");
this->set_ResidentMemoryText("Memory Res: --");
this->set_CommittedMemoryText("Memory Com: --");
this->set_Drawables2DText("Drawables 2D: --");
this->set_Drawables3DText("Drawables 3D: -- DrawCalls: --");
}

std::string DebugComponent::ResolveCommittedMemoryText(::RuntimeMemoryCounters* memoryCounters)
{
    if (memoryCounters == nullptr)
    {
return "Memory Com: --";    }
return String::Concat("Memory Com: ", this->FormatMegabytes(static_cast<uint64_t>(memoryCounters->CommittedBytes)));}

std::string DebugComponent::ResolveResidentMemoryText(::RuntimeMemoryCounters* memoryCounters)
{
    if (memoryCounters == nullptr)
    {
return "Memory Res: --";    }
return String::Concat("Memory Res: ", this->FormatMegabytes(static_cast<uint64_t>(memoryCounters->ResidentBytes)));}

void DebugComponent::SyncAdditionalLineRows()
{
    if (this->OverlayHost == nullptr)
    {
return;    }
while (this->AdditionalLineRowHosts->get_Count() < AdditionalLineIds->get_Count()) {
::Entity *rowHost = this->CreateRowHost();
::TextComponent *textComponent = this->CreateRowTextComponent(rowHost);
this->AdditionalLineRowHosts->Add(rowHost);
this->AdditionalLineTextComponents->Add(textComponent);
}
while (this->AdditionalLineRowHosts->get_Count() > AdditionalLineIds->get_Count()) {
const int32_t lastIndex = this->AdditionalLineRowHosts->get_Count() - 1;
::Entity *rowHost = (*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lastIndex));
this->AdditionalLineRowHosts->RemoveAt(static_cast<int32_t>(lastIndex));
this->AdditionalLineTextComponents->RemoveAt(static_cast<int32_t>(lastIndex));
rowHost->Dispose();
}
}

void DebugComponent::TearDownOverlay()
{
::Entity *overlayHost = this->OverlayHost;
this->ReleaseOverlayReferences();
    if (overlayHost != nullptr)
    {
overlayHost->Dispose();
    }
}

void DebugComponent::TryRefreshOverlay()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
::Core *core = (Core::Instance != nullptr ? Core::Instance : throw new InvalidOperationException("DebugComponent requires an active Core instance."));
const double elapsedSeconds = core->TotalElapsedSeconds - this->LastSampleElapsedSeconds;
    if (this->get_RefreshIntervalSeconds() > 0.0 && elapsedSeconds < this->get_RefreshIntervalSeconds())
    {
return;    }
::RuntimeMemoryCounters *memoryCounters = nullptr;
    if (core->InitializationOptions->RuntimeDiagnosticsProvider != nullptr)
    {
memoryCounters = (this->MemoryCountersValue != nullptr ? this->MemoryCountersValue : throw new InvalidOperationException("DebugComponent requires reusable memory counters while attached."));
core->RuntimeDiagnosticsService->CaptureMemoryCounters(memoryCounters);
    }
const double safeElapsedSeconds = elapsedSeconds <= 0.0 ? 1.0 : elapsedSeconds;
const double updateFps = this->UpdateFrameCount / safeElapsedSeconds;
const double updateMilliseconds = this->UpdateFrameCount <= 0 ? Number::NaN<double>() : safeElapsedSeconds * 1000.0 / this->UpdateFrameCount;
const double renderFps = this->RenderFrameCount / safeElapsedSeconds;
this->set_UpdateFpsText(this->FormatUpdateFpsText(updateFps, updateMilliseconds));
this->set_RenderFpsText(this->FormatRenderFpsText(renderFps, core->LastRenderManager3DDrawMilliseconds));
this->set_ResidentMemoryText(this->ResolveResidentMemoryText(memoryCounters));
this->set_CommittedMemoryText(this->ResolveCommittedMemoryText(memoryCounters));
this->set_Drawables2DText(String::Concat("Drawables 2D: ", std::to_string(core->ObjectManager->Drawables2D->get_Count())));
this->set_Drawables3DText(String::Concat("Drawables 3D: ", std::to_string(core->ObjectManager->Drawables3D->get_Count()), " DrawCalls: ", std::to_string(core->LastRenderManager3DDrawCallCount)));
this->UpdatePerformanceOverlayLines(core);
this->ApplyVisibleText();
this->UpdateFrameCount = 0;
this->RenderFrameCount = 0;
this->LastSampleElapsedSeconds = core->TotalElapsedSeconds;
}

void DebugComponent::UpdatePerformanceOverlayLines(::Core* core)
{
    if (core == nullptr)
    {
throw new ArgumentNullException("core");
    }
    if (!core->UsesPerformanceOverlayMetrics)
    {
DebugComponent::ClearAdditionalLine(PerformanceOverlayPrimaryLineId);
DebugComponent::ClearAdditionalLine(PerformanceOverlaySecondaryLineId);
return;    }
DebugComponent::SetAdditionalLine(PerformanceOverlayPrimaryLineId, this->FormatPerformanceOverlayPrimaryLine(core));
DebugComponent::SetAdditionalLine(PerformanceOverlaySecondaryLineId, this->FormatPerformanceOverlaySecondaryLine(core));
}

