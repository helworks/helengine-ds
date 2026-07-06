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
    /// Verifies the Nintendo DS codegen profile defaults to the generic forced-disabled debug-overlay feature setting.
    /// </summary>
    [Fact]
    public void Create_sets_generic_forced_disabled_feature_setting_by_default() {
        PlatformDefinition definition = NintendoDsPlatformDefinitionFactory.Create();

        PlatformCodegenProfileDefinition codegenProfile = Assert.Single(definition.CodegenProfiles);
        PlatformSettingDefinition disabledFeatureSetting = Assert.Single(
            codegenProfile.Settings.Where(candidate => candidate.SettingId == PlatformCodegenSettingIds.ForcedDisabledFeatures));

        Assert.Equal(PlatformSettingKind.Text, disabledFeatureSetting.SettingKind);
        Assert.Equal("debug_overlay", disabledFeatureSetting.DefaultValue);
    }
}
