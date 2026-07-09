using helengine.baseplatform.Definitions;
using helengine.ds.builder;

namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS platform definition metadata exposed to the editor.
/// </summary>
public class NintendoDsPlatformDefinitionFactoryTests {
    /// <summary>
    /// Verifies the Nintendo DS platform definition exposes the generic text background-layer override for text components.
    /// </summary>
    [Fact]
    public void Create_exposes_text_component_background_layer_override_definition() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        PlatformComponentMemberDefinition backgroundLayerDefinition = definition.ComponentMemberDefinitions
            .Single(candidate => candidate.ComponentTypeId == "helengine.TextComponent" && candidate.MemberName == "BGLayer");

        Assert.Equal("BG Layer", backgroundLayerDefinition.DisplayName);
        Assert.Equal(PlatformComponentMemberValueKind.Int32, backgroundLayerDefinition.ValueKind);
        Assert.Equal("0", backgroundLayerDefinition.DefaultValue);
        Assert.Equal(0, backgroundLayerDefinition.Order);
    }

    /// <summary>
    /// Verifies the Nintendo DS codegen profile defaults to the generic forced-disabled debug-overlay, shader, and text-processing feature settings.
    /// </summary>
    [Fact]
    public void Create_sets_generic_forced_disabled_feature_setting_by_default() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        PlatformCodegenProfileDefinition codegenProfile = Assert.Single(definition.CodegenProfiles);
        PlatformSettingDefinition disabledFeatureSetting = Assert.Single(
            codegenProfile.Settings.Where(candidate => candidate.SettingId == PlatformCodegenSettingIds.ForcedDisabledFeatures));

        Assert.Equal(PlatformSettingKind.Text, disabledFeatureSetting.SettingKind);
        Assert.Equal("debug_overlay;shaders;text_processing", disabledFeatureSetting.DefaultValue);
    }

    /// <summary>
    /// Verifies the Nintendo DS codegen profile keeps compact native exception messages disabled by default so debug-profile builds retain descriptive failures unless a build profile opts into compaction.
    /// </summary>
    [Fact]
    public void Create_sets_compact_native_exception_message_setting_disabled_by_default() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        PlatformCodegenProfileDefinition codegenProfile = Assert.Single(definition.CodegenProfiles);
        PlatformSettingDefinition compactExceptionSetting = Assert.Single(
            codegenProfile.Settings.Where(candidate => candidate.SettingId == PlatformCodegenSettingIds.CompactNativeExceptionMessages));

        Assert.Equal(PlatformSettingKind.Boolean, compactExceptionSetting.SettingKind);
        Assert.Equal("false", compactExceptionSetting.DefaultValue);
    }

    /// <summary>
    /// Verifies the Nintendo DS release build profile opts compact native exception messages back on so size-oriented builds keep the stripped exception path.
    /// </summary>
    [Fact]
    public void Create_release_build_profile_enables_compact_native_exception_messages_via_build_profile_override() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        PlatformBuildProfileDefinition buildProfile = Assert.Single(
            definition.BuildProfiles,
            candidate => string.Equals(candidate.ProfileId, "release", StringComparison.Ordinal));

        Assert.Equal("true", buildProfile.CodegenSettingDefaultValues[PlatformCodegenSettingIds.CompactNativeExceptionMessages]);
    }

    /// <summary>
    /// Verifies both debug and release Nintendo DS build profiles continue exposing the native runtime diagnostics toggle with the shared disabled-by-default value.
    /// </summary>
    [Fact]
    public void Create_keeps_native_runtime_diagnostics_build_setting_disabled_by_default_for_all_build_profiles() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        Assert.Equal(2, definition.BuildProfiles.Length);
        for (int index = 0; index < definition.BuildProfiles.Length; index++) {
            PlatformBuildProfileDefinition buildProfile = definition.BuildProfiles[index];
            PlatformSettingDefinition runtimeDiagnosticsSetting = Assert.Single(
                buildProfile.Settings.Where(candidate => candidate.SettingId == "enable-native-runtime-diagnostics"));

            Assert.Equal(PlatformSettingKind.Boolean, runtimeDiagnosticsSetting.SettingKind);
            Assert.Equal("false", runtimeDiagnosticsSetting.DefaultValue);
        }
    }
}
