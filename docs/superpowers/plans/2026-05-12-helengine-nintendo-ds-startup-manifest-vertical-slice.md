# Helengine Nintendo DS Startup Manifest Vertical Slice Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first end-to-end Nintendo DS builder and player slice that packages a builder-owned startup manifest into NitroFS and uses it to change the DS runtime screen colors at boot.

**Architecture:** The implementation follows the established Windows and PS2 split. A new managed builder under `builder/` owns DS platform metadata, startup-manifest serialization, workspace creation, and native build orchestration, while the native DS host under `src/platform/ds` mounts NitroFS, validates the packaged manifest, and switches from bootstrap colors to manifest-driven colors.

**Tech Stack:** .NET 9, xUnit, Helengine baseplatform contracts, Docker, devkitPro/devkitARM, GNU Make, libnds, NitroFS, C++17.

---

## File Structure

- `global.json`
  Pins the local .NET SDK so the DS builder matches the Windows and PS2 repos.
- `builder/helengine.ds.builder.csproj`
  Managed builder project that references `helengine.baseplatform` and `helengine.files`.
- `builder/helengine.ds.builder.sln`
  Small local solution for the DS builder and its tests.
- `builder/AssemblyInfo.cs`
  Exposes internals to the builder test project.
- `builder/Program.cs`
  Mirrors the Windows and PS2 `--describe` and `--smoke-test` entrypoint pattern.
- `builder/NintendoDsPlatformAssetBuilder.cs`
  DS implementation of `IPlatformAssetBuilder`.
- `builder/NintendoDsPlatformDefinitionFactory.cs`
  DS platform metadata exposed to the editor.
- `builder/NintendoDsStartupManifest.cs`
  Builder-owned startup-manifest payload model.
- `builder/NintendoDsStartupManifestWriter.cs`
  Serializes the startup manifest into the staged NitroFS runtime path.
- `builder/INintendoDsNativeBuildExecutor.cs`
  Test seam for the native DS build invocation.
- `builder/NintendoDsBuildWorkspace.cs`
  Resolves repository, staging, NitroFS, and output paths for one DS build.
- `builder/NintendoDsNativeBuildExecutor.cs`
  Invokes the Docker-backed native DS packaging flow and copies the final `.nds` to the requested output root.
- `builder.tests/helengine.ds.builder.tests.csproj`
  Test project for the DS builder.
- `builder.tests/Builders/RecordingDiagnosticReporter.cs`
  Shared test helper mirroring the Windows test project.
- `builder.tests/Builders/RecordingProgressReporter.cs`
  Shared test helper mirroring the Windows test project.
- `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`
  Covers DS builder metadata and `BuildAsync()` orchestration.
- `builder.tests/NintendoDsStartupManifestWriterTests.cs`
  Covers manifest byte layout and staged NitroFS output path creation.
- `builder.tests/NintendoDsBuildWorkspaceTests.cs`
  Covers DS build workspace path resolution.
- `src/platform/ds/NintendoDsStartupManifestReader.hpp`
  Declares the DS-side manifest reader and its nested result contract.
- `src/platform/ds/NintendoDsStartupManifestReader.cpp`
  Mount-independent file parsing and validation for the packaged NitroFS startup manifest.
- `src/platform/ds/NintendoDsBootHost.hpp`
  Adds startup-manifest loading state to the DS runtime host.
- `src/platform/ds/NintendoDsBootHost.cpp`
  Mounts NitroFS, loads the startup manifest, and switches colors after bootstrap initialization.
- `Makefile`
  Accepts a staged NitroFS root and packages it into the `.nds`.
- `README.md`
  Documents the builder-owned NitroFS flow and the new manifest-driven boot verification.

### Task 1: Create the DS Builder Scaffold And Metadata Contract

**Files:**
- Create: `global.json`
- Create: `builder/helengine.ds.builder.csproj`
- Create: `builder/helengine.ds.builder.sln`
- Create: `builder/AssemblyInfo.cs`
- Create: `builder/Program.cs`
- Create: `builder/NintendoDsPlatformAssetBuilder.cs`
- Create: `builder/NintendoDsPlatformDefinitionFactory.cs`
- Create: `builder.tests/helengine.ds.builder.tests.csproj`
- Create: `builder.tests/Builders/RecordingDiagnosticReporter.cs`
- Create: `builder.tests/Builders/RecordingProgressReporter.cs`
- Create: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Create the .NET scaffold and the first failing metadata test**

```json
{
  "sdk": {
    "version": "9.0.111",
    "rollForward": "latestFeature"
  }
}
```

```xml
<!-- builder/helengine.ds.builder.csproj -->
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net9.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>disable</Nullable>
    <OutputType>Exe</OutputType>
    <HelengineRoot Condition="'$(HelengineRoot)' == '' and '$(HELENGINE_ROOT)' != ''">$(HELENGINE_ROOT)</HelengineRoot>
    <HelengineRoot Condition="'$(HelengineRoot)' == ''">$(MSBuildThisFileDirectory)..\..\helengine</HelengineRoot>
  </PropertyGroup>

  <ItemGroup>
    <ProjectReference Include="$(HelengineRoot)\engine\helengine.baseplatform\helengine.baseplatform.csproj" />
    <ProjectReference Include="$(HelengineRoot)\engine\helengine.files\helengine.files.csproj" />
  </ItemGroup>

  <Target Name="ValidateHelengineRoot" BeforeTargets="CollectPackageReferences">
    <Error Condition="!Exists('$(HelengineRoot)\engine\helengine.baseplatform\helengine.baseplatform.csproj')" Text="helengine checkout not found. Set the HelengineRoot MSBuild property or HELENGINE_ROOT environment variable to a valid helengine checkout." />
  </Target>
</Project>
```

```xml
<!-- builder.tests/helengine.ds.builder.tests.csproj -->
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>net9.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>disable</Nullable>
    <IsPackable>false</IsPackable>
    <HelengineRoot Condition="'$(HelengineRoot)' == '' and '$(HELENGINE_ROOT)' != ''">$(HELENGINE_ROOT)</HelengineRoot>
    <HelengineRoot Condition="'$(HelengineRoot)' == ''">$(MSBuildThisFileDirectory)..\..\helengine</HelengineRoot>
  </PropertyGroup>

  <ItemGroup>
    <PackageReference Include="coverlet.collector" Version="6.0.2" />
    <PackageReference Include="Microsoft.NET.Test.Sdk" Version="17.11.1" />
    <PackageReference Include="xunit" Version="2.9.0" />
    <PackageReference Include="xunit.runner.visualstudio" Version="2.8.2" />
  </ItemGroup>

  <ItemGroup>
    <Using Include="Xunit" />
  </ItemGroup>

  <ItemGroup>
    <ProjectReference Include="..\builder\helengine.ds.builder.csproj" />
    <ProjectReference Include="$(HelengineRoot)\engine\helengine.baseplatform\helengine.baseplatform.csproj" />
  </ItemGroup>

  <Target Name="ValidateHelengineRoot" BeforeTargets="CollectPackageReferences">
    <Error Condition="!Exists('$(HelengineRoot)\engine\helengine.baseplatform\helengine.baseplatform.csproj')" Text="helengine checkout not found. Set the HelengineRoot MSBuild property or HELENGINE_ROOT environment variable to a valid helengine checkout." />
  </Target>
</Project>
```

```csharp
// builder/AssemblyInfo.cs
using System.Runtime.CompilerServices;

[assembly: InternalsVisibleTo("helengine.ds.builder.tests")]
```

```csharp
// builder/Program.cs
namespace helengine.ds.builder;

/// <summary>
/// Provides a small command-line entrypoint for the Nintendo DS builder assembly.
/// </summary>
public static class Program {
    /// <summary>
    /// Prints builder metadata or runs the builder smoke mode.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>Zero on success.</returns>
    public static int Main(string[] args) {
        if (args.Length > 0 && string.Equals(args[0], "--describe", StringComparison.OrdinalIgnoreCase)) {
            NintendoDsPlatformAssetBuilder builder = new();
            Console.WriteLine(builder.Descriptor.BuilderId);
            Console.WriteLine(builder.Descriptor.TargetPlatformId);
            Console.WriteLine(builder.Definition.DisplayName);
            return 0;
        }

        if (args.Length > 0 && string.Equals(args[0], "--smoke-test", StringComparison.OrdinalIgnoreCase)) {
            Console.WriteLine("ds.builder smoke test entrypoint");
            return 0;
        }

        Console.WriteLine("helengine.ds.builder --describe | --smoke-test");
        return 0;
    }
}
```

```csharp
// builder.tests/Builders/RecordingProgressReporter.cs
using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.ds.builder.tests.Builders;

public sealed class RecordingProgressReporter : IPlatformBuildProgressReporter {
    public List<PlatformBuildProgressUpdate> Updates { get; } = [];

    public void Report(PlatformBuildProgressUpdate update) {
        Updates.Add(update);
    }
}
```

```csharp
// builder.tests/Builders/RecordingDiagnosticReporter.cs
using helengine.baseplatform.Builders;
using helengine.baseplatform.Reporting;

namespace helengine.ds.builder.tests.Builders;

public sealed class RecordingDiagnosticReporter : IPlatformBuildDiagnosticReporter {
    public List<PlatformBuildDiagnostic> Diagnostics { get; } = [];

    public void Report(PlatformBuildDiagnostic diagnostic) {
        Diagnostics.Add(diagnostic);
    }
}
```

```csharp
// builder.tests/NintendoDsPlatformAssetBuilderTests.cs
using helengine.baseplatform.Definitions;

namespace helengine.ds.builder.tests;

public class NintendoDsPlatformAssetBuilderTests {
    [Fact]
    public void Descriptor_and_definition_expose_ds_metadata() {
        NintendoDsPlatformAssetBuilder builder = new();

        Assert.Equal("helengine.ds.builder", builder.Descriptor.BuilderId);
        Assert.Equal("ds", builder.Descriptor.TargetPlatformId);
        Assert.Equal("ds", builder.Definition.PlatformId);
        Assert.Contains(builder.Definition.BuildProfiles, profile => profile.ProfileId == "ds-default");
        Assert.Contains(builder.Definition.GraphicsProfiles, profile => profile.ProfileId == "ds-main-2d");
        Assert.Contains(builder.Definition.StorageProfiles, profile =>
            profile.ProfileId == "nitrofs-package" &&
            profile.RuntimeSpecializationId == "ds-nitrofs-package");

        PlatformBuildProfileDefinition buildProfile = Assert.Single(
            builder.Definition.BuildProfiles,
            profile => profile.ProfileId == "ds-default");
        Assert.Contains(buildProfile.Settings, setting => setting.SettingId == "startup-top-screen-color");
        Assert.Contains(buildProfile.Settings, setting => setting.SettingId == "startup-bottom-screen-color");

        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "helengine.meshcomponent" &&
            supportRule.SupportKind == PlatformComponentSupportKind.Transform);
        Assert.Contains(builder.Definition.ComponentSupportRules, supportRule =>
            supportRule.ComponentTypeId == "helengine.fpscomponent" &&
            supportRule.SupportKind == PlatformComponentSupportKind.Transform);
    }
}
```

- [ ] **Step 2: Create the builder solution and run the metadata test to verify it fails**

Run:

```bash
rtk dotnet new sln --name helengine.ds.builder --output builder
rtk dotnet sln builder/helengine.ds.builder.sln add builder/helengine.ds.builder.csproj builder.tests/helengine.ds.builder.tests.csproj
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine
```

Expected:

```text
FAIL with C# compile errors because NintendoDsPlatformAssetBuilder does not exist yet.
```

- [ ] **Step 3: Implement the minimal DS builder metadata**

```csharp
// builder/NintendoDsPlatformDefinitionFactory.cs
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Profiles;

namespace helengine.ds.builder;

/// <summary>
/// Creates the typed platform metadata exposed by the Nintendo DS builder.
/// </summary>
public static class NintendoDsPlatformDefinitionFactory {
    /// <summary>
    /// Creates the initial Nintendo DS platform definition for the startup-manifest slice.
    /// </summary>
    /// <returns>Typed DS platform metadata.</returns>
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
            [],
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
            ]);
    }
}
```

```csharp
// builder/NintendoDsPlatformAssetBuilder.cs
using helengine.baseplatform.Builders;
using helengine.baseplatform.Definitions;
using helengine.baseplatform.Descriptors;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Results;

namespace helengine.ds.builder;

/// <summary>
/// Implements the Nintendo DS platform asset builder contract.
/// </summary>
public sealed class NintendoDsPlatformAssetBuilder : IPlatformAssetBuilder {
    readonly INintendoDsNativeBuildExecutor NativeBuildExecutor;

    /// <summary>
    /// Initializes the builder with the default native build executor.
    /// </summary>
    public NintendoDsPlatformAssetBuilder() {
        NativeBuildExecutor = new NintendoDsNativeBuildExecutor();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Initializes the builder with a custom executor for tests.
    /// </summary>
    /// <param name="nativeBuildExecutor">Custom native build executor.</param>
    internal NintendoDsPlatformAssetBuilder(INintendoDsNativeBuildExecutor nativeBuildExecutor) {
        NativeBuildExecutor = nativeBuildExecutor ?? throw new ArgumentNullException(nameof(nativeBuildExecutor));
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    /// <summary>
    /// Gets the builder descriptor exposed to the editor.
    /// </summary>
    public PlatformBuilderDescriptor Descriptor { get; }

    /// <summary>
    /// Gets the DS platform definition exposed to the editor.
    /// </summary>
    public PlatformDefinition Definition { get; }

    /// <summary>
    /// Material cooking is not part of the startup-manifest slice.
    /// </summary>
    /// <param name="request">Material request.</param>
    /// <returns>No result because material cooking is not implemented in this slice.</returns>
    public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
        throw new NotSupportedException("Nintendo DS material cooking is not part of the startup-manifest slice.");
    }

    /// <summary>
    /// Build orchestration is added in a later task of this plan.
    /// </summary>
    public Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        throw new NotSupportedException("Nintendo DS build orchestration is added in a later task of this plan.");
    }

    static PlatformBuilderDescriptor CreateDescriptor() {
        return new PlatformBuilderDescriptor(
            "helengine.ds.builder",
            "1.0.0",
            "ds",
            new EngineCompatibilityRange("1.0.0", "999.0.0"),
            new ManifestCompatibilityRange(1, 3),
            ["ds"],
            ["ds-default"]);
    }
}
```

```csharp
// builder/INintendoDsNativeBuildExecutor.cs
namespace helengine.ds.builder;

/// <summary>
/// Represents one native Nintendo DS build executor.
/// </summary>
public interface INintendoDsNativeBuildExecutor {
    /// <summary>
    /// Builds the native DS package for one workspace.
    /// </summary>
    /// <param name="workspace">Workspace to build.</param>
    /// <param name="cancellationToken">Cancellation token.</param>
    void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken);
}
```

```csharp
// builder/NintendoDsBuildWorkspace.cs
namespace helengine.ds.builder;

/// <summary>
/// Placeholder build workspace populated in a later task.
/// </summary>
public sealed class NintendoDsBuildWorkspace {
}
```

```csharp
// builder/NintendoDsNativeBuildExecutor.cs
namespace helengine.ds.builder;

/// <summary>
/// Placeholder native build executor populated in a later task.
/// </summary>
public sealed class NintendoDsNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    public void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken) {
        throw new NotSupportedException("Nintendo DS native build execution is added in a later task of this plan.");
    }
}
```

- [ ] **Step 4: Run the metadata test to verify it passes**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine --filter FullyQualifiedName~NintendoDsPlatformAssetBuilderTests.Descriptor_and_definition_expose_ds_metadata
```

Expected:

```text
PASS with 1 test passed.
```

- [ ] **Step 5: Commit the scaffold and metadata contract**

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add global.json builder builder.tests
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add DS builder scaffold and metadata"
```

### Task 2: Add The Startup Manifest Contract And Writer

**Files:**
- Create: `builder/NintendoDsStartupManifest.cs`
- Create: `builder/NintendoDsStartupManifestWriter.cs`
- Create: `builder.tests/NintendoDsStartupManifestWriterTests.cs`
- Modify: `builder/NintendoDsPlatformAssetBuilder.cs`

- [ ] **Step 1: Write the failing startup-manifest writer tests**

```csharp
// builder.tests/NintendoDsStartupManifestWriterTests.cs
namespace helengine.ds.builder.tests;

public class NintendoDsStartupManifestWriterTests {
    [Fact]
    public void Write_creates_expected_manifest_bytes() {
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-manifest-" + Guid.NewGuid().ToString("N"));
        try {
            NintendoDsStartupManifestWriter writer = new();
            string nitroFsRootPath = Path.Combine(workingRoot, "nitrofs");

            string manifestPath = writer.Write(
                nitroFsRootPath,
                topScreenColorHex: "#FF0000",
                bottomScreenColorHex: "#0000FF");

            byte[] bytes = File.ReadAllBytes(manifestPath);

            Assert.Equal(Path.Combine(nitroFsRootPath, "runtime", "ds_startup_manifest.bin"), manifestPath);
            Assert.Equal(new byte[] {
                0x48, 0x44, 0x53, 0x50,
                0x01, 0x00,
                0x04, 0x00,
                0x1F, 0x80,
                0x00, 0xFC
            }, bytes);
        } finally {
            if (Directory.Exists(workingRoot)) {
                Directory.Delete(workingRoot, recursive: true);
            }
        }
    }

    [Fact]
    public void Write_when_color_text_is_invalid_throws() {
        NintendoDsStartupManifestWriter writer = new();
        string nitroFsRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-manifest-invalid-" + Guid.NewGuid().ToString("N"));

        try {
            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
                writer.Write(nitroFsRootPath, "#12", "#0000FF"));
            Assert.Contains("startup top screen color", exception.Message, StringComparison.OrdinalIgnoreCase);
        } finally {
            if (Directory.Exists(nitroFsRootPath)) {
                Directory.Delete(nitroFsRootPath, recursive: true);
            }
        }
    }
}
```

- [ ] **Step 2: Run the manifest writer tests to verify they fail**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine --filter FullyQualifiedName~NintendoDsStartupManifestWriterTests
```

Expected:

```text
FAIL with compile errors because NintendoDsStartupManifestWriter does not exist yet.
```

- [ ] **Step 3: Implement the builder-owned manifest model and writer**

```csharp
// builder/NintendoDsStartupManifest.cs
namespace helengine.ds.builder;

/// <summary>
/// Stores the DS-native startup manifest payload written into NitroFS.
/// </summary>
public sealed class NintendoDsStartupManifest {
    /// <summary>
    /// Initializes one manifest with already-packed DS screen colors.
    /// </summary>
    /// <param name="topScreenColor">Packed top-screen color.</param>
    /// <param name="bottomScreenColor">Packed bottom-screen color.</param>
    public NintendoDsStartupManifest(ushort topScreenColor, ushort bottomScreenColor) {
        TopScreenColor = topScreenColor;
        BottomScreenColor = bottomScreenColor;
    }

    /// <summary>
    /// Gets the packed top-screen color.
    /// </summary>
    public ushort TopScreenColor { get; }

    /// <summary>
    /// Gets the packed bottom-screen color.
    /// </summary>
    public ushort BottomScreenColor { get; }
}
```

```csharp
// builder/NintendoDsStartupManifestWriter.cs
namespace helengine.ds.builder;

/// <summary>
/// Writes the builder-owned DS startup manifest into a staged NitroFS root.
/// </summary>
public sealed class NintendoDsStartupManifestWriter {
    const string ManifestRelativePath = "runtime/ds_startup_manifest.bin";

    /// <summary>
    /// Writes one startup manifest into the staged NitroFS root.
    /// </summary>
    /// <param name="nitroFsRootPath">NitroFS root directory.</param>
    /// <param name="topScreenColorHex">Top-screen color in <c>#RRGGBB</c> form.</param>
    /// <param name="bottomScreenColorHex">Bottom-screen color in <c>#RRGGBB</c> form.</param>
    /// <returns>Full path to the emitted manifest.</returns>
    public string Write(string nitroFsRootPath, string topScreenColorHex, string bottomScreenColorHex) {
        if (string.IsNullOrWhiteSpace(nitroFsRootPath)) {
            throw new ArgumentException("NitroFS root path must be provided.", nameof(nitroFsRootPath));
        }

        NintendoDsStartupManifest manifest = new(
            ParseColor(topScreenColorHex, "startup top screen color"),
            ParseColor(bottomScreenColorHex, "startup bottom screen color"));

        string manifestPath = Path.Combine(nitroFsRootPath, "runtime", "ds_startup_manifest.bin");
        Directory.CreateDirectory(Path.GetDirectoryName(manifestPath)!);

        using FileStream stream = File.Create(manifestPath);
        using BinaryWriter writer = new(stream);
        writer.Write(new byte[] { 0x48, 0x44, 0x53, 0x50 });
        writer.Write((ushort)1);
        writer.Write((ushort)4);
        writer.Write(manifest.TopScreenColor);
        writer.Write(manifest.BottomScreenColor);

        return manifestPath;
    }

    static ushort ParseColor(string colorText, string fieldName) {
        if (string.IsNullOrWhiteSpace(colorText)) {
            throw new InvalidOperationException($"The {fieldName} build setting is required.");
        }

        string normalized = colorText.Trim();
        if (normalized.StartsWith("#", StringComparison.Ordinal)) {
            normalized = normalized.Substring(1);
        }

        if (normalized.Length != 6) {
            throw new InvalidOperationException($"The {fieldName} build setting must use #RRGGBB format.");
        }

        byte red = Convert.ToByte(normalized.Substring(0, 2), 16);
        byte green = Convert.ToByte(normalized.Substring(2, 2), 16);
        byte blue = Convert.ToByte(normalized.Substring(4, 2), 16);

        ushort packedRed = (ushort)((red * 31) / 255);
        ushort packedGreen = (ushort)(((green * 31) / 255) << 5);
        ushort packedBlue = (ushort)(((blue * 31) / 255) << 10);
        return (ushort)(0x8000 | packedRed | packedGreen | packedBlue);
    }
}
```

```csharp
// builder/NintendoDsPlatformAssetBuilder.cs
// Replace only the cook method with this implementation detail and keep BuildAsync throwing for now.
public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
    throw new NotSupportedException("Nintendo DS material cooking is not part of the startup-manifest slice.");
}
```

- [ ] **Step 4: Run the manifest writer tests to verify they pass**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine --filter FullyQualifiedName~NintendoDsStartupManifestWriterTests
```

Expected:

```text
PASS with 2 tests passed.
```

- [ ] **Step 5: Commit the startup-manifest contract**

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add builder/NintendoDsStartupManifest.cs builder/NintendoDsStartupManifestWriter.cs builder.tests/NintendoDsStartupManifestWriterTests.cs
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add DS startup manifest writer"
```

### Task 3: Add DS Build Workspace And BuildAsync Orchestration

**Files:**
- Create: `builder/NintendoDsBuildWorkspace.cs`
- Modify: `builder/NintendoDsNativeBuildExecutor.cs`
- Modify: `builder/NintendoDsPlatformAssetBuilder.cs`
- Create: `builder.tests/NintendoDsBuildWorkspaceTests.cs`
- Modify: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing workspace and build orchestration tests**

```csharp
// builder.tests/NintendoDsBuildWorkspaceTests.cs
namespace helengine.ds.builder.tests;

public class NintendoDsBuildWorkspaceTests {
    [Fact]
    public void Create_builds_expected_staging_and_output_paths() {
        string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
        string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-work-" + Guid.NewGuid().ToString("N"));
        string outputRoot = Path.Combine(workingRoot, "out");
        string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            repositoryRoot,
            workingRoot,
            outputRoot,
            generatedCoreRoot);

        Assert.Equal(repositoryRoot, workspace.RepositoryRootPath);
        Assert.Equal(Path.Combine(workingRoot, "ds", "nitrofs"), workspace.NitroFsRootPath);
        Assert.Equal("/workspace-staging/ds/nitrofs", workspace.ContainerNitroFsRootPath);
        Assert.Equal(Path.Combine(repositoryRoot, "build", "helengine_ds.nds"), workspace.RepositoryPackagePath);
        Assert.Equal(Path.Combine(outputRoot, "helengine_ds.nds"), workspace.ExportPackagePath);
    }
}
```

```csharp
// Add to builder.tests/NintendoDsPlatformAssetBuilderTests.cs
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Profiles;
using helengine.baseplatform.Reporting;
using helengine.baseplatform.Requests;
using helengine.baseplatform.Targets;
using helengine.ds.builder.tests.Builders;

[Fact]
public async Task BuildAsync_writes_startup_manifest_and_invokes_native_executor() {
    string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
    string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
    string outputRoot = Path.Combine(workingRoot, "out");
    string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
    Directory.CreateDirectory(generatedCoreRoot);

    try {
        RecordingDiagnosticReporter diagnosticReporter = new();
        RecordingProgressReporter progressReporter = new();
        FakeNativeBuildExecutor nativeBuildExecutor = new();
        NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

        PlatformBuildManifest manifest = new(
            3,
            "project",
            "1.0.0",
            "1.0.0",
            "startup",
            [],
            Array.Empty<PlatformBuildAsset>(),
            Array.Empty<PlatformBuildArtifact>(),
            Array.Empty<PlatformBuildCodeModule>(),
            Array.Empty<PlatformArtifactPlacement>(),
            new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

        PlatformBuildRequest request = new(
            manifest,
            [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
            [new PlatformCookProfile(
                "ds-default",
                "DS Default",
                new PlatformCookProfileCapabilities(
                    "ds",
                    "raw",
                    "raw",
                    "ds-scene-v1",
                    PlatformSerializationEndianness.LittleEndian))],
            outputRoot,
            Path.Combine(workingRoot, "tmp"),
            selectedBuildProfileId: "ds-default",
            selectedGraphicsProfileId: "ds-main-2d",
            selectedCodegenProfileId: "default",
            selectedBuildOptionValues: new Dictionary<string, string> {
                ["startup-top-screen-color"] = "#FF0000",
                ["startup-bottom-screen-color"] = "#0000FF"
            },
            selectedGraphicsOptionValues: new Dictionary<string, string>(),
            selectedCodegenOptionValues: new Dictionary<string, string>(),
            generatedCoreCppRootPath: generatedCoreRoot,
            selectedMediaProfileId: "ds-cartridge",
            selectedStorageProfileId: "nitrofs-package");

        var report = await builder.BuildAsync(request, progressReporter, diagnosticReporter, CancellationToken.None);

        Assert.True(report.Succeeded);
        Assert.NotNull(nativeBuildExecutor.Workspace);
        Assert.True(File.Exists(Path.Combine(nativeBuildExecutor.Workspace.NitroFsRootPath, "runtime", "ds_startup_manifest.bin")));
        Assert.True(File.Exists(nativeBuildExecutor.Workspace.ExportPackagePath));
        Assert.Empty(diagnosticReporter.Diagnostics);
    } finally {
        if (Directory.Exists(workingRoot)) {
            Directory.Delete(workingRoot, recursive: true);
        }
    }
}

sealed class FakeNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    public NintendoDsBuildWorkspace Workspace { get; private set; }

    public void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken) {
        Workspace = workspace;
        Directory.CreateDirectory(Path.GetDirectoryName(workspace.ExportPackagePath)!);
        File.WriteAllText(workspace.ExportPackagePath, "nds");
    }
}
```

- [ ] **Step 2: Run the workspace and build tests to verify they fail**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine --filter FullyQualifiedName~NintendoDsBuildWorkspaceTests|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests.BuildAsync_writes_startup_manifest_and_invokes_native_executor
```

Expected:

```text
FAIL with compile errors because NintendoDsBuildWorkspace.Create and the NintendoDsPlatformAssetBuilder(repositoryRoot) constructor do not exist yet.
```

- [ ] **Step 3: Implement the workspace, native executor, and BuildAsync**

```csharp
// builder/NintendoDsBuildWorkspace.cs
namespace helengine.ds.builder;

/// <summary>
/// Stores the resolved file-system paths for one Nintendo DS build.
/// </summary>
public sealed class NintendoDsBuildWorkspace {
    NintendoDsBuildWorkspace(
        string repositoryRootPath,
        string workingRootPath,
        string outputRootPath,
        string generatedCoreRootPath,
        string nitroFsRootPath,
        string containerNitroFsRootPath,
        string repositoryPackagePath,
        string exportPackagePath) {
        RepositoryRootPath = repositoryRootPath;
        WorkingRootPath = workingRootPath;
        OutputRootPath = outputRootPath;
        GeneratedCoreRootPath = generatedCoreRootPath;
        NitroFsRootPath = nitroFsRootPath;
        ContainerNitroFsRootPath = containerNitroFsRootPath;
        RepositoryPackagePath = repositoryPackagePath;
        ExportPackagePath = exportPackagePath;
    }

    public string RepositoryRootPath { get; }
    public string WorkingRootPath { get; }
    public string OutputRootPath { get; }
    public string GeneratedCoreRootPath { get; }
    public string NitroFsRootPath { get; }
    public string ContainerNitroFsRootPath { get; }
    public string RepositoryPackagePath { get; }
    public string ExportPackagePath { get; }

    public static NintendoDsBuildWorkspace Create(
        string repositoryRootPath,
        string workingRootPath,
        string outputRootPath,
        string generatedCoreRootPath) {
        if (string.IsNullOrWhiteSpace(repositoryRootPath)) {
            throw new ArgumentException("Repository root path must be provided.", nameof(repositoryRootPath));
        }

        string nitroFsRootPath = Path.Combine(workingRootPath, "ds", "nitrofs");
        string repositoryPackagePath = Path.Combine(repositoryRootPath, "build", "helengine_ds.nds");
        string exportPackagePath = Path.Combine(outputRootPath, "helengine_ds.nds");
        return new NintendoDsBuildWorkspace(
            repositoryRootPath,
            workingRootPath,
            outputRootPath,
            generatedCoreRootPath,
            nitroFsRootPath,
            "/workspace-staging/ds/nitrofs",
            repositoryPackagePath,
            exportPackagePath);
    }
}
```

```csharp
// builder/NintendoDsNativeBuildExecutor.cs
using System.Diagnostics;

namespace helengine.ds.builder;

/// <summary>
/// Invokes the Docker-backed native Nintendo DS packaging build.
/// </summary>
public sealed class NintendoDsNativeBuildExecutor : INintendoDsNativeBuildExecutor {
    public void Build(NintendoDsBuildWorkspace workspace, CancellationToken cancellationToken) {
        if (workspace == null) {
            throw new ArgumentNullException(nameof(workspace));
        }

        Directory.CreateDirectory(workspace.OutputRootPath);
        Directory.CreateDirectory(workspace.NitroFsRootPath);

        ProcessStartInfo startInfo = new ProcessStartInfo {
            FileName = "docker",
            WorkingDirectory = workspace.RepositoryRootPath,
            RedirectStandardOutput = true,
            RedirectStandardError = true
        };
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--rm");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(workspace.RepositoryRootPath + ":/workspace");
        startInfo.ArgumentList.Add("-v");
        startInfo.ArgumentList.Add(workspace.WorkingRootPath + ":/workspace-staging");
        startInfo.ArgumentList.Add("-w");
        startInfo.ArgumentList.Add("/workspace");
        startInfo.ArgumentList.Add("helengine-ds");
        startInfo.ArgumentList.Add("make");
        startInfo.ArgumentList.Add("HELENGINE_DS_NITROFS_ROOT=" + workspace.ContainerNitroFsRootPath);

        using Process process = Process.Start(startInfo)
            ?? throw new InvalidOperationException("Unable to start the Nintendo DS Docker build.");

        process.WaitForExit();
        if (process.ExitCode != 0) {
            string standardError = process.StandardError.ReadToEnd();
            throw new InvalidOperationException("Nintendo DS Docker build failed: " + standardError);
        }

        if (!File.Exists(workspace.RepositoryPackagePath)) {
            throw new InvalidOperationException("Nintendo DS package output was not produced.");
        }

        File.Copy(workspace.RepositoryPackagePath, workspace.ExportPackagePath, true);
    }
}
```

```csharp
// builder/NintendoDsPlatformAssetBuilder.cs
using helengine.baseplatform.Manifest;
using helengine.baseplatform.Targets;

public sealed class NintendoDsPlatformAssetBuilder : IPlatformAssetBuilder {
    readonly INintendoDsNativeBuildExecutor NativeBuildExecutor;
    readonly string RepositoryRootPath;
    readonly NintendoDsStartupManifestWriter StartupManifestWriter;

    public NintendoDsPlatformAssetBuilder() {
        NativeBuildExecutor = new NintendoDsNativeBuildExecutor();
        RepositoryRootPath = ResolveRepositoryRootPath();
        StartupManifestWriter = new NintendoDsStartupManifestWriter();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    internal NintendoDsPlatformAssetBuilder(INintendoDsNativeBuildExecutor nativeBuildExecutor, string repositoryRootPath) {
        NativeBuildExecutor = nativeBuildExecutor ?? throw new ArgumentNullException(nameof(nativeBuildExecutor));
        RepositoryRootPath = string.IsNullOrWhiteSpace(repositoryRootPath)
            ? throw new ArgumentException("Repository root path must be provided.", nameof(repositoryRootPath))
            : repositoryRootPath;
        StartupManifestWriter = new NintendoDsStartupManifestWriter();
        Descriptor = CreateDescriptor();
        Definition = NintendoDsPlatformDefinitionFactory.Create();
    }

    public async Task<PlatformBuildReport> BuildAsync(
        PlatformBuildRequest request,
        IPlatformBuildProgressReporter progressReporter,
        IPlatformBuildDiagnosticReporter diagnosticReporter,
        CancellationToken cancellationToken) {
        if (request == null) {
            throw new ArgumentNullException(nameof(request));
        }
        if (progressReporter == null) {
            throw new ArgumentNullException(nameof(progressReporter));
        }
        if (diagnosticReporter == null) {
            throw new ArgumentNullException(nameof(diagnosticReporter));
        }

        string topScreenColor = ReadRequiredBuildOption(request.SelectedBuildOptionValues, "startup-top-screen-color");
        string bottomScreenColor = ReadRequiredBuildOption(request.SelectedBuildOptionValues, "startup-bottom-screen-color");

        Directory.CreateDirectory(request.OutputRoot);
        Directory.CreateDirectory(request.WorkingRoot);

        NintendoDsBuildWorkspace workspace = NintendoDsBuildWorkspace.Create(
            RepositoryRootPath,
            request.WorkingRoot,
            request.OutputRoot,
            request.GeneratedCoreCppRootPath);

        StartupManifestWriter.Write(workspace.NitroFsRootPath, topScreenColor, bottomScreenColor);
        progressReporter.Report(new PlatformBuildProgressUpdate(
            "Write Startup Manifest",
            "runtime/ds_startup_manifest.bin",
            1,
            2,
            "Wrote the DS startup manifest into the staged NitroFS root."));

        NativeBuildExecutor.Build(workspace, cancellationToken);
        progressReporter.Report(new PlatformBuildProgressUpdate(
            "Build Nintendo DS Package",
            "helengine_ds.nds",
            2,
            2,
            "Built the Nintendo DS package."));

        PlatformBuildReport report = new(
            succeeded: File.Exists(workspace.ExportPackagePath),
            diagnostics: [],
            sceneOutcomes: [],
            looseAssetOutcomes: []);
        return await Task.FromResult(report);
    }

    static string ReadRequiredBuildOption(IReadOnlyDictionary<string, string> values, string key) {
        if (values == null) {
            throw new ArgumentNullException(nameof(values));
        }
        if (!values.TryGetValue(key, out string value) || string.IsNullOrWhiteSpace(value)) {
            throw new InvalidOperationException($"Missing required Nintendo DS build option '{key}'.");
        }
        return value;
    }

    static string ResolveRepositoryRootPath() {
        string assemblyDirectory = Path.GetDirectoryName(typeof(NintendoDsPlatformAssetBuilder).Assembly.Location)
            ?? throw new InvalidOperationException("Unable to resolve the Nintendo DS builder assembly location.");
        return Path.GetFullPath(Path.Combine(assemblyDirectory, "..", ".."));
    }
}
```

- [ ] **Step 4: Run the builder tests to verify the orchestration passes**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine
```

Expected:

```text
PASS with the DS builder metadata, manifest writer, workspace, and BuildAsync tests all green.
```

- [ ] **Step 5: Commit the DS build orchestration**

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add builder/NintendoDsBuildWorkspace.cs builder/NintendoDsNativeBuildExecutor.cs builder/NintendoDsPlatformAssetBuilder.cs builder.tests/NintendoDsBuildWorkspaceTests.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Add DS builder build orchestration"
```

### Task 4: Package NitroFS And Load The Startup Manifest In The DS Runtime

**Files:**
- Create: `src/platform/ds/NintendoDsStartupManifestReader.hpp`
- Create: `src/platform/ds/NintendoDsStartupManifestReader.cpp`
- Modify: `src/platform/ds/NintendoDsBootHost.hpp`
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
- Modify: `Makefile`

- [ ] **Step 1: Capture the failing integration behavior before changing the runtime**

Run:

```bash
mkdir -p /tmp/helengine-ds-manual/runtime
printf '\x48\x44\x53\x50\x01\x00\x04\x00\x1F\x80\x00\xFC' > /tmp/helengine-ds-manual/runtime/ds_startup_manifest.bin
rtk docker run --rm -v "$PWD":/workspace -v /tmp/helengine-ds-manual:/workspace-staging -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging
```

Expected:

```text
The package builds, but booting it in melonDS or DeSmuME still shows the old green/cyan bootstrap frame because the current Makefile and runtime ignore NitroFS startup data.
```

- [ ] **Step 2: Implement NitroFS packaging in the Makefile**

```makefile
DEVKITPRO ?= /opt/devkitpro
DEVKITARM ?= $(DEVKITPRO)/devkitARM
LIBNDS ?= $(DEVKITPRO)/libnds
HELENGINE_CORE_CPP_ROOT ?=
HELENGINE_DS_NITROFS_ROOT ?=

ifneq ($(strip $(HELENGINE_DS_NITROFS_ROOT)),)
NITROFSDIR := $(abspath $(HELENGINE_DS_NITROFS_ROOT))
$(if $(wildcard $(NITROFSDIR)),,$(error HELENGINE_DS_NITROFS_ROOT '$(HELENGINE_DS_NITROFS_ROOT)' does not exist))
endif

include $(DEVKITARM)/ds_rules
```

```makefile
# Keep the rest of the current file intact and add this export near the other exports.
ifneq ($(strip $(HELENGINE_DS_NITROFS_ROOT)),)
export NITROFSDIR := $(NITROFSDIR)
endif
```

- [ ] **Step 3: Implement the DS-side startup-manifest reader and host integration**

```cpp
// src/platform/ds/NintendoDsStartupManifestReader.hpp
#pragma once

#include <nds.h>

namespace helengine::ds {
    /// Owns DS startup-manifest parsing from NitroFS.
    class NintendoDsStartupManifestReader {
    public:
        /// Identifies one manifest read outcome.
        enum class Status {
            Success,
            MountFailed,
            FileMissing,
            InvalidMagic,
            UnsupportedVersion,
            InvalidPayloadSize,
            ReadFailed
        };

        /// Stores one manifest read result.
        struct Result {
            /// Stores the manifest read outcome.
            Status ReadStatus;

            /// Stores the resolved top-screen color when the read succeeds.
            u16 TopScreenColor;

            /// Stores the resolved bottom-screen color when the read succeeds.
            u16 BottomScreenColor;
        };

        /// Reads the packaged startup manifest from NitroFS.
        Result Read() const;
    };
}
```

```cpp
// src/platform/ds/NintendoDsStartupManifestReader.cpp
#include "platform/ds/NintendoDsStartupManifestReader.hpp"

#include <cstdio>

namespace helengine::ds {
    namespace {
        constexpr char StartupManifestPath[] = "nitro:/runtime/ds_startup_manifest.bin";
        constexpr unsigned StartupManifestVersion = 1;
        constexpr unsigned StartupManifestPayloadSize = 4;
    }

    NintendoDsStartupManifestReader::Result NintendoDsStartupManifestReader::Read() const {
        if (!nitroFSInit(nullptr)) {
            return Result { Status::MountFailed, 0, 0 };
        }

        FILE* file = fopen(StartupManifestPath, "rb");
        if (file == nullptr) {
            return Result { Status::FileMissing, 0, 0 };
        }

        unsigned char magic[4];
        if (fread(magic, 1, sizeof(magic), file) != sizeof(magic)) {
            fclose(file);
            return Result { Status::ReadFailed, 0, 0 };
        }

        if (magic[0] != 0x48 || magic[1] != 0x44 || magic[2] != 0x53 || magic[3] != 0x50) {
            fclose(file);
            return Result { Status::InvalidMagic, 0, 0 };
        }

        unsigned short version = 0;
        unsigned short payloadSize = 0;
        unsigned short topScreenColor = 0;
        unsigned short bottomScreenColor = 0;

        if (fread(&version, sizeof(version), 1, file) != 1 ||
            fread(&payloadSize, sizeof(payloadSize), 1, file) != 1 ||
            fread(&topScreenColor, sizeof(topScreenColor), 1, file) != 1 ||
            fread(&bottomScreenColor, sizeof(bottomScreenColor), 1, file) != 1) {
            fclose(file);
            return Result { Status::ReadFailed, 0, 0 };
        }

        fclose(file);

        if (version != StartupManifestVersion) {
            return Result { Status::UnsupportedVersion, 0, 0 };
        } else if (payloadSize != StartupManifestPayloadSize) {
            return Result { Status::InvalidPayloadSize, 0, 0 };
        }

        return Result { Status::Success, topScreenColor, bottomScreenColor };
    }
}
```

```cpp
// src/platform/ds/NintendoDsBootHost.hpp
#pragma once

#include <nds.h>

#include "platform/ds/NintendoDsStartupManifestReader.hpp"

namespace helengine::ds {
    /// Owns the first Nintendo DS native video bootstrap and verification frame loop.
    class NintendoDsBootHost {
    public:
        NintendoDsBootHost();
        int Run();

    private:
        static constexpr int FrameBufferWidth = 256;
        static constexpr int FrameBufferHeight = 256;
        static constexpr int FrameBufferPixelCount = FrameBufferWidth * FrameBufferHeight;
        static constexpr u16 BootstrapTopScreenColor = RGB15(0, 31, 0) | BIT(15);
        static constexpr u16 BootstrapBottomScreenColor = RGB15(0, 31, 31) | BIT(15);

        int MainBackgroundId;
        int SubBackgroundId;
        u16* MainFrameBuffer;
        u16* SubFrameBuffer;
        NintendoDsStartupManifestReader StartupManifestReader;
        NintendoDsStartupManifestReader::Status StartupManifestStatus;

        bool InitializeVideo();
        void PaintScreenColors(u16 topScreenColor, u16 bottomScreenColor);
        void TryApplyStartupManifestColors();
    };
}
```

```cpp
// src/platform/ds/NintendoDsBootHost.cpp
#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>

namespace helengine::ds {
    NintendoDsBootHost::NintendoDsBootHost()
        : MainBackgroundId(-1)
        , SubBackgroundId(-1)
        , MainFrameBuffer(nullptr)
        , SubFrameBuffer(nullptr)
        , StartupManifestStatus(NintendoDsStartupManifestReader::Status::FileMissing) {
    }

    int NintendoDsBootHost::Run() {
        powerOn(POWER_ALL_2D);

        if (!InitializeVideo()) {
            return 1;
        }

        PaintScreenColors(BootstrapTopScreenColor, BootstrapBottomScreenColor);
        TryApplyStartupManifestColors();

        while (true) {
            swiWaitForVBlank();
        }

        return 0;
    }

    bool NintendoDsBootHost::InitializeVideo() {
        videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

        vramSetBankA(VRAM_A_MAIN_BG);
        vramSetBankC(VRAM_C_SUB_BG);

        MainBackgroundId = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        SubBackgroundId = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

        if (MainBackgroundId < 0) {
            return false;
        } else if (SubBackgroundId < 0) {
            return false;
        }

        MainFrameBuffer = static_cast<u16*>(bgGetGfxPtr(MainBackgroundId));
        SubFrameBuffer = static_cast<u16*>(bgGetGfxPtr(SubBackgroundId));

        if (MainFrameBuffer == nullptr) {
            return false;
        } else if (SubFrameBuffer == nullptr) {
            return false;
        }

        return true;
    }

    void NintendoDsBootHost::PaintScreenColors(u16 topScreenColor, u16 bottomScreenColor) {
        std::fill_n(MainFrameBuffer, FrameBufferPixelCount, topScreenColor);
        std::fill_n(SubFrameBuffer, FrameBufferPixelCount, bottomScreenColor);
        swiWaitForVBlank();
    }

    void NintendoDsBootHost::TryApplyStartupManifestColors() {
        NintendoDsStartupManifestReader::Result result = StartupManifestReader.Read();
        StartupManifestStatus = result.ReadStatus;
        if (result.ReadStatus != NintendoDsStartupManifestReader::Status::Success) {
            return;
        }

        PaintScreenColors(result.TopScreenColor, result.BottomScreenColor);
    }
}
```

- [ ] **Step 4: Rebuild and verify the runtime now uses the packaged manifest**

Run:

```bash
rtk docker run --rm -v "$PWD":/workspace -v /tmp/helengine-ds-manual:/workspace-staging -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging
```

Expected:

```text
The package rebuilds successfully. Booting build/helengine_ds.nds in melonDS or DeSmuME now shows a red top screen and a blue bottom screen instead of the green/cyan bootstrap frame.
```

- [ ] **Step 5: Commit the native NitroFS runtime integration**

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add Makefile src/platform/ds/NintendoDsStartupManifestReader.hpp src/platform/ds/NintendoDsStartupManifestReader.cpp src/platform/ds/NintendoDsBootHost.hpp src/platform/ds/NintendoDsBootHost.cpp
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Load DS startup manifest from NitroFS"
```

### Task 5: Update Docs And Run The Final Verification Set

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Update the README to document the builder-owned NitroFS path**

````markdown
# Helengine Nintendo DS Host

This repository contains the native Nintendo DS host scaffold and Nintendo DS platform builder for Helengine.

## Current milestone

- Docker-only build using devkitPro Nintendo DS tooling
- Managed DS builder scaffold under `builder/`
- Native `.nds` output for direct loading in melonDS or DeSmuME
- Builder-owned startup manifest packaged through NitroFS
- First packaged boot check with manifest-driven screen colors

## Native build

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

The native build emits `build/helengine_ds.nds`.

## Builder tests

```bash
dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine
```

## NitroFS startup manifest check

The DS builder writes `runtime/ds_startup_manifest.bin` into a staged NitroFS root and the runtime loads it from `nitro:/runtime/ds_startup_manifest.bin`.

To test the native package manually without the editor, create a staged NitroFS root with a manifest and pass it into the Docker build:

```bash
mkdir -p /tmp/helengine-ds-manual/runtime
printf '\x48\x44\x53\x50\x01\x00\x04\x00\x1F\x80\x00\xFC' > /tmp/helengine-ds-manual/runtime/ds_startup_manifest.bin
docker run --rm -v "$PWD":/workspace -v /tmp/helengine-ds-manual:/workspace-staging -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging
```

Boot `build/helengine_ds.nds` in melonDS or DeSmuME. The expected result is a red top screen and a blue bottom screen. If the manifest is missing or invalid, the runtime remains on the green/cyan bootstrap frame.
````

- [ ] **Step 2: Run the builder tests and the native Docker build together**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine
rtk docker run --rm -v "$PWD":/workspace -v /tmp/helengine-ds-manual:/workspace-staging -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging
```

Expected:

```text
All DS builder tests pass, and the Docker build emits build/helengine_ds.nds successfully.
```

- [ ] **Step 3: Run the two manual emulator checks**

1. Boot the manifest-driven package from Step 2 and confirm the runtime frame is red on top and blue on bottom.
2. Delete `/tmp/helengine-ds-manual/runtime/ds_startup_manifest.bin`, rebuild with the same Docker command, boot again, and confirm the runtime stays on the green/cyan bootstrap frame.

Expected:

```text
The first boot proves packaged manifest loading, and the second boot proves the explicit bootstrap fallback path.
```

- [ ] **Step 4: Commit the README and verification update**

```bash
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds add README.md
rtk git -c safe.directory=/mnt/c/dev/helworks/helengine-ds commit -m "Document DS startup manifest packaging"
```

## Self-Review

- Spec coverage:
  - Builder scaffold and metadata are covered by Task 1.
  - Binary startup manifest path, layout, and validation are covered by Task 2 and Task 4.
  - Build workspace and native packaging handoff are covered by Task 3.
  - NitroFS packaging and runtime color switching are covered by Task 4.
  - README and manual emulator verification are covered by Task 5.
- Placeholder scan:
  - No `TODO`, `TBD`, or “implement later” placeholders remain.
  - Each task contains concrete file paths, test cases, commands, and commit steps.
- Type consistency:
  - The builder id is consistently `helengine.ds.builder`.
  - The platform id is consistently `ds`.
  - The NitroFS manifest path is consistently `runtime/ds_startup_manifest.bin` and `nitro:/runtime/ds_startup_manifest.bin`.
  - The build-profile setting ids are consistently `startup-top-screen-color` and `startup-bottom-screen-color`.
