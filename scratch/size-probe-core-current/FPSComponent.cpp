#ifdef DrawText
#undef DrawText
#endif
#include "FPSComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "UpdateComponent.hpp"
#include "Entity.hpp"
#include "runtime/native_list.hpp"
#include "FPSComponent.hpp"
#include "NativeOwnership.hpp"
#include "Core.hpp"
#include "runtime/native_string.hpp"
#include "TextComponent.hpp"
#include "system/math.hpp"
#include "int2.hpp"
#include "byte4.hpp"
#include "float3.hpp"
#include "FontAsset.hpp"
#include "float4.hpp"
#include "float4x4.hpp"
#include "Component.hpp"
#include "runtime/array.hpp"
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
#include "RuntimeTexture.hpp"
#include "TextAlignment.hpp"
#include "RoundedRectComponent.hpp"
#include "TextComponentSelectionUpdateComponent.hpp"
#include "float2.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_string.hpp"
#include "system/diagnostics/stopwatch.hpp"

int32_t FPSComponent::get_ActiveComponentCount()
{
return ActiveComponents->get_Count();
}

double FPSComponent::get_RefreshIntervalSeconds()
{
return this->refreshIntervalSeconds;}

void FPSComponent::set_RefreshIntervalSeconds(double value)
{
    if (value < 0.0)
    {
throw ([&]() {
auto __ctor_arg_000001C2 = "value";
auto __ctor_arg_000001C3 = "Refresh interval must be zero or greater.";
return new ArgumentOutOfRangeException(__ctor_arg_000001C2, __ctor_arg_000001C3);
})();
    }
this->refreshIntervalSeconds = value;
}

::int2 FPSComponent::get_Padding()
{
return this->padding;}

void FPSComponent::set_Padding(::int2 value)
{
this->padding = value;
this->ApplyPadding();
}

uint8_t FPSComponent::get_RenderOrder2D()
{
return this->renderOrder2D;}

void FPSComponent::set_RenderOrder2D(uint8_t value)
{
    if (this->renderOrder2D == value)
    {
return;    }
this->renderOrder2D = value;
this->ApplyRenderOrder();
}

::FontAsset* FPSComponent::get_Font()
{
return this->font;}

void FPSComponent::set_Font(::FontAsset* value)
{
    if ((this->font == value))
    {
return;    }
this->font = value;
this->RefreshOverlayActivation();
}

float FPSComponent::get_FontScale()
{
return this->FontScaleValue;}

void FPSComponent::set_FontScale(float value)
{
    if (value <= 0.0f)
    {
throw ([&]() {
auto __ctor_arg_000001C4 = "value";
auto __ctor_arg_000001C5 = "Font scale must be greater than zero.";
return new ArgumentOutOfRangeException(__ctor_arg_000001C4, __ctor_arg_000001C5);
})();
    }
    if (this->FontScaleValue == value)
    {
return;    }
this->FontScaleValue = value;
this->ApplyFont();
}

const std::string& FPSComponent::get_AdditionalText()
{
return this->AdditionalTextValue;}

void FPSComponent::set_AdditionalText(std::string value)
{
    if (String::IsNullOrEmpty(value))
    {
value = String::Empty;
    }
    if (this->AdditionalTextValue == value)
    {
return;    }
this->AdditionalTextValue = value;
this->UpdateAdditionalLineRows(Core::Instance);
}

const std::string& FPSComponent::get_UpdateFpsText()
{
return this->UpdateFpsText;
}

void FPSComponent::set_UpdateFpsText(std::string value)
{
this->UpdateFpsText = value;
}

const std::string& FPSComponent::get_RenderFpsText()
{
return this->RenderFpsText;
}

void FPSComponent::set_RenderFpsText(std::string value)
{
this->RenderFpsText = value;
}

const std::string& FPSComponent::get_DetailFpsText()
{
return this->DetailFpsText;
}

void FPSComponent::set_DetailFpsText(std::string value)
{
this->DetailFpsText = value;
}

void FPSComponent::ComponentAdded(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
UpdateComponent::ComponentAdded(entity);
this->RefreshOverlayActivation();
}

void FPSComponent::ComponentRemoved(::Entity* entity)
{
this->TearDownOverlay();
UpdateComponent::ComponentRemoved(entity);
}

void FPSComponent::Dispose()
{
this->TearDownOverlay();
UpdateComponent::Dispose();
}

FPSComponent::FPSComponent() : UpdateFpsText(), RenderFpsText(), DetailFpsText(), font(), FontScaleValue(1.0f), AdditionalTextValue(String::Empty), VisibleAdditionalTextValue(String::Empty), OverlayHost(), UpdateRowHost(), RenderRowHost(), UpdateTextComponent(), RenderTextComponent(), AdditionalLineRowHosts(), AdditionalLineTextComponents(), LastSampleElapsedSeconds(0), UpdateFrameCount(0), RenderFrameCount(0), Initialized(), refreshIntervalSeconds(0.5), padding(::int2(static_cast<int32_t>(8), static_cast<int32_t>(6))), renderOrder2D(250)
{
this->AdditionalLineRowHosts = new List<::Entity*>();
this->AdditionalLineTextComponents = new List<::TextComponent*>();
this->ResetSamplingWindow();
}

void FPSComponent::ParentEnabledChange(bool newEnabled)
{
UpdateComponent::ParentEnabledChange(newEnabled);
this->ResetSamplingWindow();
this->ApplyCurrentOverlayText();
}

void FPSComponent::RecordRenderFrame()
{
for (int32_t i = ActiveComponents->get_Count() - 1; i >= 0; i--) {
::FPSComponent *component = (*ActiveComponents).get_Item(static_cast<int32_t>(i));
    if (component->Initialized && component->Parent != nullptr && component->Parent->get_IsHierarchyEnabled())
    {
component->RenderFrameCount++;
    }
}
}

void FPSComponent::RecordUpdateFrame()
{
for (int32_t i = ActiveComponents->get_Count() - 1; i >= 0; i--) {
::FPSComponent *component = (*ActiveComponents).get_Item(static_cast<int32_t>(i));
    if (component->Initialized && component->Parent != nullptr && component->Parent->get_IsHierarchyEnabled())
    {
component->UpdateFrameCount++;
    }
}
}

void FPSComponent::Update()
{
    if (!this->Initialized)
    {
return;    }
this->TryRefreshOverlay();
}

uint8_t FPSComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void FPSComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* FPSComponent::get_Parent()
{
return Component::get_Parent();
}

void FPSComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool FPSComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* FPSComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool FPSComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

List<::FPSComponent*>* FPSComponent::ActiveComponents = new List<::FPSComponent*>();

void FPSComponent::AppendAdditionalLineRow(std::string lineText)
{
    if (static_cast<int32_t>(lineText.size()) == 0)
    {
return;    }
::Entity *rowHost = new ::Entity();
rowHost->set_LayerMask(this->Parent->get_LayerMask());
rowHost->InitChildren();
rowHost->InitComponents();
this->OverlayHost->AddChild(rowHost);
::TextComponent *textComponent = new ::TextComponent();
textComponent->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
textComponent->set_Font(this->get_Font());
textComponent->set_FontScale(this->get_FontScale());
textComponent->set_Text(lineText);
rowHost->AddComponent(textComponent);
this->AdditionalLineRowHosts->Add(rowHost);
this->AdditionalLineTextComponents->Add(textComponent);
}

void FPSComponent::ApplyCurrentOverlayText()
{
    if (!this->Initialized)
    {
return;    }
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
::Core *core = Core::Instance;
    if (core == nullptr)
    {
return;    }
this->set_UpdateFpsText(this->ResolveUpdateOverlayText(core, 0.0));
this->set_RenderFpsText(this->ResolveRenderOverlayText(core, 0.0, core->LastRenderManager3DDrawMilliseconds));
this->set_DetailFpsText(String::Empty);
    if (this->UpdateTextComponent != nullptr)
    {
this->UpdateTextComponent->set_Text(this->UpdateFpsText);
    }
    if (this->RenderTextComponent != nullptr)
    {
this->RenderTextComponent->set_Text(this->FormatOverlaySecondaryLine(this->RenderFpsText));
    }
this->UpdateAdditionalLineRows(core);
this->PublishResolvedOverlayTextRows(core);
this->ApplyRowLayout();
}

void FPSComponent::ApplyFont()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->UpdateTextComponent != nullptr)
    {
this->UpdateTextComponent->set_Font(this->get_Font());
this->UpdateTextComponent->set_FontScale(this->get_FontScale());
    }
    if (this->RenderTextComponent != nullptr)
    {
this->RenderTextComponent->set_Font(this->get_Font());
this->RenderTextComponent->set_FontScale(this->get_FontScale());
    }
this->ApplyRowLayout();
}

void FPSComponent::ApplyOverlayPresentationVisibility()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
::Core *core = Core::Instance;
this->OverlayHost->set_Enabled(!this->ShouldUsePlatformOwnedOverlayPresentation(core));
}

void FPSComponent::ApplyPadding()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->OverlayHost == nullptr)
    {
return;    }
this->OverlayHost->set_LocalPosition(::float3(this->padding.X, this->padding.Y, 0.0f));
}

void FPSComponent::ApplyRenderOrder()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
    if (this->UpdateTextComponent != nullptr)
    {
this->UpdateTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
    if (this->RenderTextComponent != nullptr)
    {
this->RenderTextComponent->set_RenderOrder2D(this->get_RenderOrder2D());
    }
for (int32_t lineIndex = 0; lineIndex < this->AdditionalLineTextComponents->get_Count(); lineIndex++) {
(*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(lineIndex))->set_RenderOrder2D(this->get_RenderOrder2D());
}
}

void FPSComponent::ApplyRowLayout()
{
    if (this->get_Font() == nullptr)
    {
return;    }
const float rowHeight = this->get_Font()->LineHeight * this->get_FontScale();
    if (this->RenderRowHost != nullptr)
    {
this->RenderRowHost->set_LocalPosition(::float3(0.0f, rowHeight, 0.1f));
    }
for (int32_t lineIndex = 0; lineIndex < this->AdditionalLineRowHosts->get_Count(); lineIndex++) {
(*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex))->set_LocalPosition(::float3(0.0f, rowHeight * (lineIndex + 2), 0.1f));
}
}

bool FPSComponent::AreAdditionalRowsLive()
{
    if (this->AdditionalLineRowHosts->get_Count() != this->AdditionalLineTextComponents->get_Count())
    {
return false;    }
for (int32_t lineIndex = 0; lineIndex < this->AdditionalLineRowHosts->get_Count(); lineIndex++) {
    if (!this->IsLiveRow((*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex)), this->OverlayHost, (*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(lineIndex))))
    {
return false;    }
}
return true;}

void FPSComponent::BuildOverlay()
{
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("FPSComponent must be attached before its overlay can be created.");
    }
    if (this->get_Font() == nullptr)
    {
throw new InvalidOperationException("FPSComponent overlay creation requires a font.");
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
this->UpdateRowHost = new ::Entity();
this->UpdateRowHost->set_LayerMask(this->Parent->get_LayerMask());
this->UpdateRowHost->InitChildren();
this->UpdateRowHost->InitComponents();
this->OverlayHost->AddChild(this->UpdateRowHost);
this->UpdateTextComponent = new ::TextComponent();
this->UpdateTextComponent->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
this->UpdateTextComponent->set_Font(this->get_Font());
this->UpdateTextComponent->set_FontScale(this->get_FontScale());
this->UpdateRowHost->AddComponent(this->UpdateTextComponent);
this->RenderRowHost = new ::Entity();
this->RenderRowHost->set_LayerMask(this->Parent->get_LayerMask());
this->RenderRowHost->InitChildren();
this->RenderRowHost->InitComponents();
this->OverlayHost->AddChild(this->RenderRowHost);
this->RenderTextComponent = new ::TextComponent();
this->RenderTextComponent->set_Color(::byte4(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255)));
this->RenderTextComponent->set_Font(this->get_Font());
this->RenderTextComponent->set_FontScale(this->get_FontScale());
this->RenderRowHost->AddComponent(this->RenderTextComponent);
this->Initialized = true;
ActiveComponents->Add(this);
this->ApplyFont();
this->UpdateAdditionalLineRows(Core::Instance);
this->ApplyOverlayPresentationVisibility();
}

List<std::string>* FPSComponent::BuildVisibleAdditionalLines()
{
List<std::string> *visibleLines = new List<std::string>();
    if (String::IsNullOrWhiteSpace(this->VisibleAdditionalTextValue))
    {
return visibleLines;    }
std::string currentLine = String::Empty;
for (int32_t characterIndex = 0; characterIndex < static_cast<int32_t>(this->VisibleAdditionalTextValue.size()); characterIndex++) {
const char currentCharacter = this->VisibleAdditionalTextValue[characterIndex];
    if (currentCharacter == '\r')
    {
continue;
    }
    if (currentCharacter == '\n')
    {
    if (static_cast<int32_t>(currentLine.size()) > 0)
    {
visibleLines->Add(currentLine);
    }
currentLine = String::Empty;
continue;
    }
currentLine += currentCharacter;
}
    if (static_cast<int32_t>(currentLine.size()) > 0)
    {
visibleLines->Add(currentLine);
    }
return visibleLines;}

void FPSComponent::ClearPublishedOverlayTextRows()
{
::Core *core = Core::Instance;
    if (core == nullptr)
    {
return;    }
core->SetResolvedPerformanceOverlayPresentation(nullptr, 1.0f, ::int2(static_cast<int32_t>(0), static_cast<int32_t>(0)), String::Empty, String::Empty, String::Empty, String::Empty);
}

void FPSComponent::DisposeAdditionalLineRows()
{
for (int32_t lineIndex = this->AdditionalLineRowHosts->get_Count() - 1; lineIndex >= 0; lineIndex--) {
if ((*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex)) != nullptr)
{
(*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex))->Dispose();
delete (*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex));
}
}
this->AdditionalLineRowHosts->Clear();
this->AdditionalLineTextComponents->Clear();
}

bool FPSComponent::EnsureOverlayHierarchyIsLive()
{
    if (!this->Initialized)
    {
return false;    }
return this->OverlayHost != nullptr && !this->OverlayHost->get_IsDisposed() && this->OverlayHost->get_ParentUnsafe() == this->Parent && this->IsLiveRow(this->UpdateRowHost, this->OverlayHost, this->UpdateTextComponent) && this->IsLiveRow(this->RenderRowHost, this->OverlayHost, this->RenderTextComponent) && this->AreAdditionalRowsLive();}

std::string FPSComponent::FormatFpsValue(double fps)
{
const int32_t tenths = static_cast<int32_t>(Math::Round(fps * 10.0, static_cast<MidpointRounding>(MidpointRounding::AwayFromZero)));
const int32_t whole = tenths / 10;
const int32_t fractional = Math::Abs(static_cast<int32_t>(tenths % 10));
return String::Concat(std::to_string(whole), ".", std::to_string(fractional));}

std::string FPSComponent::FormatOverlaySecondaryLine(std::string baseRenderText)
{
return baseRenderText;}

std::string FPSComponent::FormatRenderFpsText(double renderFps, double drawMilliseconds)
{
return String::Concat("Render FPS: ", this->FormatFpsValue(renderFps), " (", this->FormatFpsValue(drawMilliseconds), " ms)");}

bool FPSComponent::IsLiveRow(::Entity* rowHost, ::Entity* overlayHost, ::TextComponent* textComponent)
{
return rowHost != nullptr && !rowHost->get_IsDisposed() && rowHost->get_ParentUnsafe() == overlayHost && textComponent != nullptr && !textComponent->get_IsDisposed() && textComponent->get_ParentUnsafe() == rowHost;}

void FPSComponent::PublishResolvedOverlayTextRows(::Core* core)
{
    if (core == nullptr)
    {
return;    }
core->SetResolvedPerformanceOverlayPresentation(this->get_Font(), this->get_FontScale(), this->get_Padding(), this->UpdateFpsText, this->RenderFpsText, this->DetailFpsText, this->VisibleAdditionalTextValue);
}

void FPSComponent::RefreshOverlayActivation()
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
this->ApplyCurrentOverlayText();
this->ApplyPadding();
this->ApplyRenderOrder();
this->ApplyOverlayPresentationVisibility();
return;    }
this->ApplyFont();
this->ApplyRenderOrder();
this->ApplyPadding();
this->ApplyOverlayPresentationVisibility();
}

void FPSComponent::ReleaseOverlayReferences()
{
ActiveComponents->Remove(this);
this->OverlayHost = nullptr;
this->UpdateRowHost = nullptr;
this->RenderRowHost = nullptr;
this->UpdateTextComponent = nullptr;
this->RenderTextComponent = nullptr;
this->AdditionalLineRowHosts->Clear();
this->AdditionalLineTextComponents->Clear();
this->VisibleAdditionalTextValue = String::Empty;
this->Initialized = false;
}

void FPSComponent::RemoveAdditionalLineRowAt(int32_t lineIndex)
{
::Entity *rowHost = (*this->AdditionalLineRowHosts).get_Item(static_cast<int32_t>(lineIndex));
this->AdditionalLineRowHosts->RemoveAt(static_cast<int32_t>(lineIndex));
this->AdditionalLineTextComponents->RemoveAt(static_cast<int32_t>(lineIndex));
if (rowHost != nullptr)
{
rowHost->Dispose();
delete rowHost;
}
}

void FPSComponent::ResetSamplingWindow()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
    }
this->UpdateFrameCount = 0;
this->RenderFrameCount = 0;
::Core *core = Core::Instance;
this->LastSampleElapsedSeconds = core == nullptr ? 0.0 : core->TotalElapsedSeconds;
    if (this->Initialized && core != nullptr)
    {
this->set_UpdateFpsText(this->ResolveUpdateOverlayText(core, 0.0));
this->set_RenderFpsText(this->ResolveRenderOverlayText(core, 0.0, core->LastRenderManager3DDrawMilliseconds));
this->set_DetailFpsText(String::Empty);
    }
else {
this->set_UpdateFpsText("Update FPS: --");
this->set_RenderFpsText("Render FPS: -- (-- ms)");
this->set_DetailFpsText(String::Empty);
}
    if (this->UpdateTextComponent != nullptr)
    {
this->UpdateTextComponent->set_Text(this->UpdateFpsText);
    }
    if (this->RenderTextComponent != nullptr)
    {
this->RenderTextComponent->set_Text(this->FormatOverlaySecondaryLine(this->RenderFpsText));
    }
this->UpdateAdditionalLineRows(core);
this->PublishResolvedOverlayTextRows(core);
this->ApplyRowLayout();
}

std::string FPSComponent::ResolveAdditionalOverlayText(::Core* core)
{
return String::Empty;}

std::string FPSComponent::ResolveDetailOverlayText(::Core* core)
{
return String::Empty;}

std::string FPSComponent::ResolveRenderOverlayText(::Core* core, double renderFps, double drawMilliseconds)
{
    if (this->ShouldUsePlatformOwnedOverlayTextRows(core))
    {
    if (!String::IsNullOrEmpty(core->PerformanceOverlayRenderText))
    {
return core->PerformanceOverlayRenderText;    }
return String::Concat("Rdr ", this->FormatFpsValue(renderFps), " Drw ", this->FormatFpsValue(drawMilliseconds));    }
    if (this->ShouldUsePerformanceOverlayRows(core))
    {
return String::Concat("Rdr ", this->FormatFpsValue(renderFps), " Drw ", this->FormatFpsValue(drawMilliseconds), " Enc ", this->FormatFpsValue(core->PerformanceOverlayPacketEncodeMilliseconds), " Lgt ", this->FormatFpsValue(core->PerformanceOverlaySubmitMilliseconds));    }
return this->FormatRenderFpsText(renderFps, drawMilliseconds);}

std::string FPSComponent::ResolveUpdateOverlayText(::Core* core, double updateFps)
{
    if (this->ShouldUsePlatformOwnedOverlayTextRows(core))
    {
    if (!String::IsNullOrEmpty(core->PerformanceOverlayUpdateText))
    {
return core->PerformanceOverlayUpdateText;    }
return String::Concat("Upd ", this->FormatFpsValue(updateFps));    }
    if (this->ShouldUsePerformanceOverlayRows(core))
    {
return String::Concat("Upd ", this->FormatFpsValue(updateFps), " Set ", this->FormatFpsValue(core->PerformanceOverlayTriangleSetupMilliseconds), " Prep ", this->FormatFpsValue(core->PerformanceOverlayTrianglePrepMilliseconds), " Emit ", this->FormatFpsValue(core->PerformanceOverlayTriangleEmitMilliseconds));    }
return String::Concat("Update FPS: ", this->FormatFpsValue(updateFps));}

bool FPSComponent::ShouldUsePerformanceOverlayRows(::Core* core)
{
return core != nullptr && core->UsesPerformanceOverlayMetrics;}

bool FPSComponent::ShouldUsePlatformOwnedOverlayPresentation(::Core* core)
{
return core != nullptr && core->UsesPlatformOwnedPerformanceOverlayPresentation;}

bool FPSComponent::ShouldUsePlatformOwnedOverlayTextRows(::Core* core)
{
return core != nullptr && core->UsesPerformanceOverlayMetrics && (!String::IsNullOrEmpty(core->PerformanceOverlayUpdateText) || !String::IsNullOrEmpty(core->PerformanceOverlayRenderText) || !String::IsNullOrEmpty(core->PerformanceOverlayDetailText) || !String::IsNullOrEmpty(core->PerformanceOverlayAdditionalText));}

void FPSComponent::SynchronizeAdditionalLineRows()
{
    if (!this->Initialized)
    {
return;    }
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
List<std::string> *visibleLines = this->BuildVisibleAdditionalLines();
while (this->AdditionalLineRowHosts->get_Count() > visibleLines->get_Count()) {
this->RemoveAdditionalLineRowAt(static_cast<int32_t>(this->AdditionalLineRowHosts->get_Count() - 1));
}
for (int32_t lineIndex = 0; lineIndex < visibleLines->get_Count(); lineIndex++) {
const std::string lineText = (*visibleLines).get_Item(static_cast<int32_t>(lineIndex));
    if (lineIndex >= this->AdditionalLineTextComponents->get_Count())
    {
this->AppendAdditionalLineRow(lineText);
continue;
    }
::TextComponent *textComponent = (*this->AdditionalLineTextComponents).get_Item(static_cast<int32_t>(lineIndex));
textComponent->set_Font(this->get_Font());
textComponent->set_FontScale(this->get_FontScale());
textComponent->set_Text(lineText);
}
this->ApplyRenderOrder();
this->ApplyRowLayout();
}

void FPSComponent::TearDownOverlay()
{
::Entity *overlayHost = this->OverlayHost;
this->ReleaseOverlayReferences();
this->ClearPublishedOverlayTextRows();
    if (overlayHost != nullptr)
    {
if (overlayHost != nullptr)
{
overlayHost->Dispose();
delete overlayHost;
}
    }
}

void FPSComponent::TryRefreshOverlay()
{
    if (!this->EnsureOverlayHierarchyIsLive())
    {
this->ReleaseOverlayReferences();
return;    }
::Core *core = Core::Instance;
const double elapsedSeconds = core->TotalElapsedSeconds - this->LastSampleElapsedSeconds;
    if (this->refreshIntervalSeconds > 0.0 && elapsedSeconds < this->refreshIntervalSeconds)
    {
return;    }
const double safeElapsedSeconds = elapsedSeconds <= 0.0 ? 1.0 : elapsedSeconds;
const double updateFps = this->UpdateFrameCount / safeElapsedSeconds;
const double renderFps = this->RenderFrameCount / safeElapsedSeconds;
this->set_UpdateFpsText(this->ResolveUpdateOverlayText(core, updateFps));
this->set_RenderFpsText(this->ResolveRenderOverlayText(core, renderFps, core->LastRenderManager3DDrawMilliseconds));
this->set_DetailFpsText(String::Empty);
    if (this->UpdateTextComponent != nullptr)
    {
this->UpdateTextComponent->set_Text(this->UpdateFpsText);
    }
    if (this->RenderTextComponent != nullptr)
    {
this->RenderTextComponent->set_Text(this->FormatOverlaySecondaryLine(this->RenderFpsText));
    }
this->UpdateAdditionalLineRows(core);
this->PublishResolvedOverlayTextRows(core);
this->ApplyRowLayout();
this->UpdateFrameCount = 0;
this->RenderFrameCount = 0;
this->LastSampleElapsedSeconds = core->TotalElapsedSeconds;
}

void FPSComponent::UpdateAdditionalLineRows(::Core* core)
{
const std::string resolvedAdditionalText = String::Empty;
    if (this->VisibleAdditionalTextValue == resolvedAdditionalText && this->AreAdditionalRowsLive())
    {
this->PublishResolvedOverlayTextRows(core);
return;    }
this->VisibleAdditionalTextValue = resolvedAdditionalText;
this->SynchronizeAdditionalLineRows();
this->PublishResolvedOverlayTextRows(core);
}

