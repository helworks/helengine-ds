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
class RuntimeMemoryCounters;
class Core;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_dictionary.hpp"
#include "int2.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_list.hpp"

class DebugComponent : public ::UpdateComponent
{
public:
    virtual ~DebugComponent() = default;

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

    std::string UpdateFpsText;

    const std::string& get_UpdateFpsText();
    void set_UpdateFpsText(std::string value);

    std::string RenderFpsText;

    const std::string& get_RenderFpsText();
    void set_RenderFpsText(std::string value);

    std::string ResidentMemoryText;

    const std::string& get_ResidentMemoryText();
    void set_ResidentMemoryText(std::string value);

    std::string CommittedMemoryText;

    const std::string& get_CommittedMemoryText();
    void set_CommittedMemoryText(std::string value);

    std::string Drawables2DText;

    const std::string& get_Drawables2DText();
    void set_Drawables2DText(std::string value);

    std::string Drawables3DText;

    const std::string& get_Drawables3DText();
    void set_Drawables3DText(std::string value);

    static void ClearAdditionalLine(std::string id);

    static void ClearAdditionalLines();

    void ComponentAdded(::Entity* entity);

    void ComponentRemoved(::Entity* entity);

    DebugComponent();

    void ParentEnabledChange(bool newEnabled);

    static void RecordRenderFrame();

    static void RecordUpdateFrame();

    static void SetAdditionalLine(std::string id, std::string text);

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    static const double BytesPerMegabyte;

    inline static const std::string PerformanceOverlayPrimaryLineId = "performance-overlay-primary";

    inline static const std::string PerformanceOverlaySecondaryLineId = "performance-overlay-secondary";

    static List<::DebugComponent*>* ActiveComponents;

    static List<std::string>* AdditionalLineIds;

    static Dictionary<std::string, std::string>* AdditionalLinesById;

    ::FontAsset* FontValue;

    float FontScaleValue;

    ::Entity* OverlayHost;

    ::Entity* UpdateFpsRowHost;

    ::Entity* RenderFpsRowHost;

    ::Entity* ResidentMemoryRowHost;

    ::Entity* CommittedMemoryRowHost;

    ::Entity* Drawables2DRowHost;

    ::Entity* Drawables3DRowHost;

    List<::Entity*>* AdditionalLineRowHosts;

    ::TextComponent* UpdateFpsTextComponent;

    ::TextComponent* RenderFpsTextComponent;

    ::TextComponent* ResidentMemoryTextComponent;

    ::TextComponent* CommittedMemoryTextComponent;

    ::TextComponent* Drawables2DTextComponent;

    ::TextComponent* Drawables3DTextComponent;

    List<::TextComponent*>* AdditionalLineTextComponents;

    double LastSampleElapsedSeconds;

    int32_t UpdateFrameCount;

    int32_t RenderFrameCount;

    bool Initialized;

    ::RuntimeMemoryCounters* MemoryCountersValue;

    double RefreshIntervalSecondsValue;

    ::int2 PaddingValue;

    uint8_t RenderOrder2DValue;

    void ApplyAdditionalLineText();

    void ApplyFont();

    void ApplyPadding();

    void ApplyRenderOrder();

    void ApplyVisibleText();

    bool AreAdditionalLineRowsLive();

    void BuildOverlay();

    ::Entity* CreateRowHost();

    ::TextComponent* CreateRowTextComponent(::Entity* rowHost);

    bool EnsureOverlayHierarchyIsLive();

    std::string FormatMegabytes(uint64_t bytes);

    std::string FormatOneDecimal(double value);

    std::string FormatPerformanceOverlayPrimaryLine(::Core* core);

    std::string FormatPerformanceOverlaySecondaryLine(::Core* core);

    std::string FormatRenderFpsText(double renderFps, double drawMilliseconds);

    static std::string FormatRoundedMetric(double value);

    std::string FormatUpdateFpsText(double updateFps, double updateMilliseconds);

    bool IsBaseOverlayHierarchyLive();

    bool IsLiveRow(::Entity* rowHost, ::Entity* overlayHost, ::TextComponent* textComponent);

    static void RefreshAdditionalLinesOnActiveComponents();

    void RefreshOverlayActivation();

    void ReleaseOverlayReferences();

    void ResetSamplingWindow();

    std::string ResolveCommittedMemoryText(::RuntimeMemoryCounters* memoryCounters);

    std::string ResolveResidentMemoryText(::RuntimeMemoryCounters* memoryCounters);

    void SyncAdditionalLineRows();

    void TearDownOverlay();

    void TryRefreshOverlay();

    void UpdatePerformanceOverlayLines(::Core* core);
};
