using helengine.baseplatform.Definitions;
using helengine.baseplatform.Profiles;

namespace helengine.ds.builder;

/// <summary>
/// Creates the typed Nintendo DS platform metadata exposed to the editor.
/// </summary>
public static class NintendoDsPlatformDefinitionFactory {
    /// <summary>
    /// Creates the serialized default Nintendo DS texture settings contract used when assets do not provide an explicit Nintendo DS override.
    /// </summary>
    /// <returns>Serialized default Nintendo DS texture settings.</returns>
    static string CreateDefaultSerializedTextureCookSettings() {
        return NintendoDsTextureCookSettingsSerializer.Serialize(
            256,
            TextureAssetColorFormat.Rgba4444,
            TextureAssetAlphaPrecision.A4);
    }

    /// <summary>
    /// Creates the serialized default Nintendo DS font-atlas texture settings contract used when fonts do not provide an explicit Nintendo DS override.
    /// </summary>
    /// <returns>Serialized default Nintendo DS font-atlas texture settings.</returns>
    static string CreateDefaultSerializedFontAtlasTextureCookSettings() {
        return NintendoDsTextureCookSettingsSerializer.Serialize(
            256,
            TextureAssetColorFormat.Indexed4,
            TextureAssetAlphaPrecision.Binary);
    }

    /// <summary>
    /// Creates the shared Nintendo DS build-profile settings exposed by both debug and release build flavors.
    /// </summary>
    /// <returns>Shared Nintendo DS build-profile settings.</returns>
    static PlatformSettingDefinition[] CreateBuildProfileSettings() {
        return [
            new PlatformSettingDefinition(
                "startup-top-screen-color",
                "Startup Top Screen Color",
                PlatformSettingKind.Text,
                "#FF0000",
                true,
                []),
            new PlatformSettingDefinition(
                "startup-bottom-screen-color",
                "Startup Bottom Screen Color",
                PlatformSettingKind.Text,
                "#0000FF",
                true,
                []),
            new PlatformSettingDefinition(
                "enable-native-runtime-diagnostics",
                "Enable Native Runtime Diagnostics",
                PlatformSettingKind.Boolean,
                "false",
                true,
                [])
        ];
    }

    /// <summary>
    /// Creates the release-build codegen default overrides that favor a smaller native binary.
    /// </summary>
    /// <returns>Release-build codegen default overrides.</returns>
    static IReadOnlyDictionary<string, string> CreateReleaseCodegenSettingDefaults() {
        return new Dictionary<string, string>(StringComparer.Ordinal) {
            [PlatformCodegenSettingIds.CompactNativeExceptionMessages] = "true"
        };
    }

    /// <summary>
    /// Creates the initial Nintendo DS platform definition for the startup-manifest slice.
    /// </summary>
    /// <returns>The Nintendo DS platform definition consumed by the editor.</returns>
    public static PlatformDefinition Create() {
        return new PlatformDefinition(
            "ds",
            "Nintendo DS",
            [
                new PlatformBuildProfileDefinition(
                    "release",
                    "Release",
                    "Nintendo DS startup-manifest release packaging build",
                    "ds-main-2d",
                    "default",
                    CreateBuildProfileSettings(),
                    CreateReleaseCodegenSettingDefaults()),
                new PlatformBuildProfileDefinition(
                    "debug",
                    "Debug",
                    "Nintendo DS startup-manifest debug packaging build",
                    "ds-main-2d",
                    "default",
                    CreateBuildProfileSettings())
            ],
            [
                new PlatformGraphicsProfileDefinition(
                    "ds-main-2d",
                    "DS Main 2D",
                    "Nintendo DS dual-screen 2D bootstrap profile.",
                    [
                        new PlatformSettingDefinition(
                            "screen-layout",
                            "Screen Layout",
                            PlatformSettingKind.Choice,
                            "main-top-sub-bottom",
                            true,
                            ["main-top-sub-bottom"])
                    ])
            ],
            [
                new PlatformAssetRequirementDefinition(
                    "scene",
                    "Scene",
                    false,
                    ["helen"])
            ],
            [
                new PlatformMaterialSchemaDefinition(
                    NintendoDsMaterialSchemaIds.StandardTexturedSchemaId,
                    "DS Standard Textured",
                    ["ds-main-2d"],
                    [
                        new PlatformMaterialFieldDefinition(
                            NintendoDsMaterialSchemaIds.TextureRelativePathFieldId,
                            "Texture",
                            PlatformMaterialFieldKind.Text,
                            string.Empty,
                            false,
                            []),
                        new PlatformMaterialFieldDefinition(
                            NintendoDsMaterialSchemaIds.DoubleSidedFieldId,
                            "Double Sided",
                            PlatformMaterialFieldKind.Boolean,
                            "false",
                            true,
                            []),
                        new PlatformMaterialFieldDefinition(
                            NintendoDsMaterialSchemaIds.VertexColorModeFieldId,
                            "Vertex Color",
                            PlatformMaterialFieldKind.Choice,
                            "multiply",
                            true,
                            ["multiply", "ignore"]),
                        new PlatformMaterialFieldDefinition(
                            NintendoDsMaterialSchemaIds.BaseColorFieldId,
                            "Base Color",
                            PlatformMaterialFieldKind.Color,
                            "#FFFFFFFF",
                            true,
                            []),
                        new PlatformMaterialFieldDefinition(
                            NintendoDsMaterialSchemaIds.LightingModeFieldId,
                            "Lighting",
                            PlatformMaterialFieldKind.Choice,
                            "lit",
                            true,
                            ["lit", "unlit"])
                    ])
            ],
            [
                new PlatformComponentSupportRule(
                    "helengine.meshcomponent",
                    PlatformComponentSupportKind.Transform,
                    "Mesh components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.cameracomponent",
                    PlatformComponentSupportKind.Transform,
                    "Camera components are normalized during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.fpscomponent",
                    PlatformComponentSupportKind.Transform,
                    "Font references are rewritten during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.textcomponent",
                    PlatformComponentSupportKind.Transform,
                    "Font references are rewritten during packaging.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "city.menu.MenuComponent, PhysicsSceneGeneratorHarness",
                    PlatformComponentSupportKind.PassThrough,
                    "The demo-disc menu controller relies on generic scripted-component serialization and does not require Nintendo DS-specific packaging transforms.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "city.menu.MenuPanelComponent, PhysicsSceneGeneratorHarness",
                    PlatformComponentSupportKind.PassThrough,
                    "The demo-disc menu panel metadata relies on generic scripted-component serialization and does not require Nintendo DS-specific packaging transforms.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "city.menu.MenuItemComponent, PhysicsSceneGeneratorHarness",
                    PlatformComponentSupportKind.PassThrough,
                    "The demo-disc menu item metadata relies on generic scripted-component serialization and does not require Nintendo DS-specific packaging transforms.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "city.menu.PlatformInfoTextComponent, gameplay",
                    PlatformComponentSupportKind.PassThrough,
                    "The platform info overlay binder only updates runtime text values and does not require Nintendo DS-specific packaging transforms.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "city.menu.PlatformInfoTextComponent, PhysicsSceneGeneratorHarness",
                    PlatformComponentSupportKind.PassThrough,
                    "The platform info overlay binder only updates runtime text values and does not require Nintendo DS-specific packaging transforms.",
                    string.Empty),
                new PlatformComponentSupportRule(
                    "helengine.SceneMapComponent",
                    PlatformComponentSupportKind.Transform,
                    "Scene-map components are rewritten into packaged runtime payloads so startup redirection and menu remaps deserialize correctly on Nintendo DS.",
                    string.Empty)
            ],
            [
                new PlatformCodegenProfileDefinition(
                    "default",
                    "Default",
                    "Nintendo DS C# to C++ codegen profile",
                    PlatformCodegenLanguage.Cpp,
                    PlatformSerializationEndianness.LittleEndian,
                    [
                        new PlatformSettingDefinition(
                            PlatformCodegenSettingIds.ForcedDisabledFeatures,
                            "Forced Disabled Features",
                            PlatformSettingKind.Text,
                            "debug_overlay;shaders;text_processing",
                            true,
                            []),
                        new PlatformSettingDefinition(
                            PlatformCodegenSettingIds.CompactNativeExceptionMessages,
                            "Compact Native Exception Messages",
                            PlatformSettingKind.Boolean,
                            "false",
                            true,
                            [])
                    ])
            ],
            [
                new PlatformStorageProfileDefinition(
                    "nitrofs-package",
                    "NitroFS Package",
                    PlatformStorageProfileKind.LooseFiles,
                    "ds-nitrofs-package",
                    allowContainerSegmentation: false)
            ],
            [
                new PlatformMediaProfileDefinition(
                    "ds-cartridge",
                    "Nintendo DS Cartridge",
                    PlatformMediaLayoutKind.InstallTree,
                    allowPhysicalDuplication: false,
                    preferLocalityOverDeduplication: true)
            ],
            new RuntimeGenerationContract(
                RuntimeMaterialResolutionMode.CookedPlatformOwned,
                true,
                PackagedPathPolicy.ContentRelativeOnly),
            null,
            assetCookCapabilities: [
                new PlatformAssetCookCapabilityDefinition(
                    "texture",
                    "runtime-texture",
                    PlatformAssetCookOwnershipKind.BuilderOwned,
                    "ds-texture",
                    CreateDefaultSerializedTextureCookSettings(),
                    CreateTextureFormatCapabilities()),
                new PlatformAssetCookCapabilityDefinition(
                    "font-atlas-texture",
                    "runtime-texture",
                    PlatformAssetCookOwnershipKind.BuilderOwned,
                    "ds-font-atlas-texture",
                    CreateDefaultSerializedFontAtlasTextureCookSettings(),
                    CreateTextureFormatCapabilities(),
                    ".hetex")
            ],
            componentMemberDefinitions: [
                new PlatformComponentMemberDefinition(
                    "helengine.TextComponent",
                    "BGLayer",
                    "BG Layer",
                    PlatformComponentMemberValueKind.Int32,
                    "0",
                    0)
            ]);
    }

    /// <summary>
    /// Creates the generic texture format capability metadata supported by the Nintendo DS texture cooker contract.
    /// </summary>
    /// <returns>Texture capability metadata for Nintendo DS builder-owned texture cook contracts.</returns>
    static PlatformTextureFormatCapabilityDefinition CreateTextureFormatCapabilities() {
        return new PlatformTextureFormatCapabilityDefinition(
            [
                TextureAssetColorFormat.Rgba4444.ToString(),
                TextureAssetColorFormat.Indexed4.ToString(),
                TextureAssetColorFormat.Indexed8.ToString()
            ],
            [
                TextureAssetAlphaPrecision.Binary,
                TextureAssetAlphaPrecision.A4,
                TextureAssetAlphaPrecision.A8
            ],
            [
                new PlatformTextureFormatCombinationDefinition(TextureAssetColorFormat.Rgba4444.ToString(), TextureAssetAlphaPrecision.A4),
                new PlatformTextureFormatCombinationDefinition(TextureAssetColorFormat.Indexed4.ToString(), TextureAssetAlphaPrecision.Binary),
                new PlatformTextureFormatCombinationDefinition(TextureAssetColorFormat.Indexed8.ToString(), TextureAssetAlphaPrecision.A8)
            ]);
    }
}
