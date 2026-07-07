#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class int2;
class FontAsset;
class Entity;
class TextComponent;
class Core;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_list.hpp"
#include "int2.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class FPSComponent : public ::UpdateComponent
{
public:
    virtual ~FPSComponent() = default;

    static int32_t get_ActiveComponentCount();

    double get_RefreshIntervalSeconds();

    void set_RefreshIntervalSeconds(double value);

    ::int2 get_Padding();

    void set_Padding(::int2 value);

    uint8_t get_RenderOrder2D();

    void set_RenderOrder2D(uint8_t value);

    ::FontAsset* get_Font();

    void set_Font(::FontAsset* value);

    float get_FontScale();

    void set_FontScale(float value);

    const std::string& get_AdditionalText();

    void set_AdditionalText(std::string value);

    std::string UpdateFpsText;

    const std::string& get_UpdateFpsText();
    void set_UpdateFpsText(std::string value);

    std::string RenderFpsText;

    const std::string& get_RenderFpsText();
    void set_RenderFpsText(std::string value);

    std::string DetailFpsText;

    const std::string& get_DetailFpsText();
    void set_DetailFpsText(std::string value);

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    void Dispose();

    FPSComponent();

    void ParentEnabledChange(bool newEnabled);

    static void RecordRenderFrame();

    static void RecordUpdateFrame();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    static List<::FPSComponent*>* ActiveComponents;

    ::FontAsset* font;

    float FontScaleValue;

    std::string AdditionalTextValue;

    std::string VisibleAdditionalTextValue;

    ::Entity* OverlayHost;

    ::Entity* UpdateRowHost;

    ::Entity* RenderRowHost;

    ::TextComponent* UpdateTextComponent;

    ::TextComponent* RenderTextComponent;

    List<::Entity*>* AdditionalLineRowHosts;

    List<::TextComponent*>* AdditionalLineTextComponents;

    double LastSampleElapsedSeconds;

    int32_t UpdateFrameCount;

    int32_t RenderFrameCount;

    bool Initialized;

    double refreshIntervalSeconds;

    ::int2 padding;

    uint8_t renderOrder2D;

    void AppendAdditionalLineRow(std::string lineText);

    void ApplyCurrentOverlayText();

    void ApplyFont();

    void ApplyOverlayPresentationVisibility();

    void ApplyPadding();

    void ApplyRenderOrder();

    void ApplyRowLayout();

    bool AreAdditionalRowsLive();

    void BuildOverlay();

    List<std::string>* BuildVisibleAdditionalLines();

    void ClearPublishedOverlayTextRows();

    void DisposeAdditionalLineRows();

    bool EnsureOverlayHierarchyIsLive();

    std::string FormatFpsValue(double fps);

    std::string FormatOverlaySecondaryLine(std::string baseRenderText);

    std::string FormatRenderFpsText(double renderFps, double drawMilliseconds);

    bool IsLiveRow(::Entity* rowHost, ::Entity* overlayHost, ::TextComponent* textComponent);

    void PublishResolvedOverlayTextRows(::Core* core);

    void RefreshOverlayActivation();

    void ReleaseOverlayReferences();

    void RemoveAdditionalLineRowAt(int32_t lineIndex);

    void ResetSamplingWindow();

    std::string ResolveAdditionalOverlayText(::Core* core);

    std::string ResolveDetailOverlayText(::Core* core);

    std::string ResolveRenderOverlayText(::Core* core, double renderFps, double drawMilliseconds);

    std::string ResolveUpdateOverlayText(::Core* core, double updateFps);

    bool ShouldUsePerformanceOverlayRows(::Core* core);

    bool ShouldUsePlatformOwnedOverlayPresentation(::Core* core);

    bool ShouldUsePlatformOwnedOverlayTextRows(::Core* core);

    void SynchronizeAdditionalLineRows();

    void TearDownOverlay();

    void TryRefreshOverlay();

    void UpdateAdditionalLineRows(::Core* core);
};
