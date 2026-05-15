using helengine.baseplatform.Definitions;
using helengine.baseplatform.Profiles;

namespace helengine.ds.builder;

/// <summary>
/// Creates the typed Nintendo DS platform metadata exposed to the editor.
/// </summary>
public static class NintendoDsPlatformDefinitionFactory {
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
                    "ds-default",
                    "DS Default",
                    "Nintendo DS startup-manifest packaging build",
                    "ds-main-2d",
                    "default",
                    [
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
                            [])
                    ])
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
                    string.Empty)
            ],
            [
                new PlatformCodegenProfileDefinition(
                    "default",
                    "Default",
                    "Nintendo DS C# to C++ codegen profile",
                    PlatformCodegenLanguage.Cpp,
                    PlatformSerializationEndianness.LittleEndian,
                    [])
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
                PackagedPathPolicy.ContentRelativeOnly));
    }
}
