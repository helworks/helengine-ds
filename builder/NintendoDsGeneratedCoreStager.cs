using System.Text.RegularExpressions;

namespace helengine.ds.builder;

/// <summary>
/// Copies the generated-core tree into the Nintendo DS workspace so Docker can compile it.
/// </summary>
public sealed class NintendoDsGeneratedCoreStager {
    /// <summary>
    /// Stores the generated-core relative path for the platform configuration header.
    /// </summary>
    const string PlatformConfigurationRelativePath = "helcpp_config.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime-only HLSL parser source.
    /// </summary>
    const string HlslShaderBindingParserRelativePath = "HlslShaderBindingParser.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the engine core source containing host-clock draw timing.
    /// </summary>
    const string CoreSourceRelativePath = "Core.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime feature-manifest source.
    /// </summary>
    static readonly string FeatureManifestRelativePath = Path.Combine("runtime", "feature_manifest.cpp");

    /// <summary>
    /// Stores the generated-core relative path for the runtime scene-asset resolver header.
    /// </summary>
    const string RuntimeSceneAssetReferenceResolverHeaderRelativePath = "RuntimeSceneAssetReferenceResolver.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime scene-asset resolver source.
    /// </summary>
    const string RuntimeSceneAssetReferenceResolverSourceRelativePath = "RuntimeSceneAssetReferenceResolver.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime scene-load result header.
    /// </summary>
    const string RuntimeSceneLoadResultHeaderRelativePath = "RuntimeSceneLoadResult.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime scene-load result source.
    /// </summary>
    const string RuntimeSceneLoadResultSourceRelativePath = "RuntimeSceneLoadResult.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the loaded-scene record header.
    /// </summary>
    const string LoadedSceneRecordHeaderRelativePath = "LoadedSceneRecord.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the loaded-scene record source.
    /// </summary>
    const string LoadedSceneRecordSourceRelativePath = "LoadedSceneRecord.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the scene-manager header.
    /// </summary>
    const string SceneManagerHeaderRelativePath = "SceneManager.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the scene-manager source.
    /// </summary>
    const string SceneManagerSourceRelativePath = "SceneManager.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the 3D physics world source.
    /// </summary>
    const string PhysicsWorld3DSourceRelativePath = "PhysicsWorld3D.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the generated runtime-component registration source.
    /// </summary>
    const string GeneratedRuntimeComponentRegistrationSourceRelativePath = "GeneratedRuntimeComponentDeserializerRegistration.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the generated runtime-component registration header.
    /// </summary>
    const string GeneratedRuntimeComponentRegistrationHeaderRelativePath = "GeneratedRuntimeComponentDeserializerRegistration.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime component-registry source.
    /// </summary>
    const string RuntimeComponentRegistryRelativePath = "RuntimeComponentRegistry.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime asset-id generator header.
    /// </summary>
    const string RuntimeAssetIdGeneratorHeaderRelativePath = "RuntimeAssetIdGenerator.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the runtime asset-id generator source.
    /// </summary>
    const string RuntimeAssetIdGeneratorSourceRelativePath = "RuntimeAssetIdGenerator.cpp";

    /// <summary>
    /// Stores the generated-core relative path for the native hash helper header required by newer generated physics key types.
    /// </summary>
    const string HashCodeHeaderRelativePath = "HashCode.hpp";

    /// <summary>
    /// Stores the generated-core relative path for the demo-disc light-toggle source that still assumes same-entity debug overlays.
    /// </summary>
    const string DemoDiscLightToggleComponentSourceRelativePath = "DemoDiscLightToggleComponent.cpp";

    /// <summary>
    /// Stores the registry include inserted by the editor finalization pass for generated runtime deserializers.
    /// </summary>
    const string GeneratedRuntimeComponentRegistrationInclude = "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"";

    /// <summary>
    /// Stores the registry call inserted by the editor finalization pass for generated runtime deserializers.
    /// </summary>
    const string GeneratedRuntimeComponentRegistrationCall = "RegisterGeneratedRuntimeComponentDeserializers(registry);";

    /// <summary>
    /// Stores the generated config token that incorrectly marks the staged runtime as a Windows host build.
    /// </summary>
    const string WindowsHostPlatformToken = "#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 1";

    /// <summary>
    /// Stores the generated config token that marks the staged runtime as a non-Windows-host build.
    /// </summary>
    const string NonWindowsHostPlatformToken = "#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0";

    /// <summary>
    /// Stores the generated config token that incorrectly advertises Windows as the target platform.
    /// </summary>
    const string WindowsPlatformToken = "#define HE_CPP_PLATFORM_WINDOWS 1";

    /// <summary>
    /// Stores the generated config token that removes the Windows target-platform claim for Nintendo DS builds.
    /// </summary>
    const string DisabledWindowsPlatformToken = "#define HE_CPP_PLATFORM_WINDOWS 0";

    /// <summary>
    /// Stores the generated config token that incorrectly enables shader feature branches at compile time.
    /// </summary>
    const string EnabledShadersConfigurationToken = "#define HE_CPP_FEATURE_SHADERS 1";

    /// <summary>
    /// Stores the generated config token that disables shader feature branches for Nintendo DS builds.
    /// </summary>
    const string DisabledShadersConfigurationToken = "#define HE_CPP_FEATURE_SHADERS 0";

    /// <summary>
    /// Stores the regular expression that matches one complete shader feature-manifest entry.
    /// </summary>
    static readonly Regex ShaderFeatureManifestEntryPattern = new(
        @"\{\s*HEFeature::Shaders,\s*true,\s*HEFeatureDecisionOrigin::[A-Za-z0-9_]+,\s*""Shaders""\s*\}(?<suffix>,?)",
        RegexOptions.CultureInvariant);

    /// <summary>
    /// Stores the feature-manifest token that forces shaders off for Nintendo DS runtime builds.
    /// </summary>
    const string ForcedDisabledShadersFeatureManifestToken = "{ HEFeature::Shaders, false, HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }";

    /// <summary>
    /// Stores the compatibility hash helper written when the editor-generated core does not yet emit one.
    /// </summary>
    const string HashCodeHeaderSource = """
#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

class HashCode {
public:
    template <typename TFirst, typename TSecond>
    static int32_t Combine(TFirst firstValue, TSecond secondValue) {
        std::size_t combinedHash = std::hash<TFirst>{}(firstValue);
        combinedHash ^= std::hash<TSecond>{}(secondValue) + 0x9e3779b9u + (combinedHash << 6) + (combinedHash >> 2);
        return static_cast<int32_t>(combinedHash & 0x7fffffff);
    }
};
""";

    /// <summary>
    /// Stores the regular expression that matches the generated transient material release guard that only applies to raw material assets.
    /// </summary>
    static readonly Regex RuntimeSceneResolverMaterialReleaseGuardPattern = new(
        @"auto\s+__releaseMaterialAssetGuard\s*=\s*he_cpp_make_scope_exit\(\[\&\]\(\)\s*\{\s*ReleaseTransientMaterialAsset\(materialAsset\);\s*\}\);\s*",
        RegexOptions.CultureInvariant | RegexOptions.Singleline);

    /// <summary>
    /// Stores the regular expression that matches the generated transient shader release guard used by the raw material path.
    /// </summary>
    static readonly Regex RuntimeSceneResolverShaderReleaseGuardPattern = new(
        @"auto\s+__releaseShaderAssetGuard\s*=\s*he_cpp_make_scope_exit\(\[\&\]\(\)\s*\{\s*ReleaseTransientShaderAsset\(shaderAsset\);\s*\}\);\s*",
        RegexOptions.CultureInvariant | RegexOptions.Singleline);

    /// <summary>
    /// Stores the stale generated include that references one header no longer emitted by current editor finalization.
    /// </summary>
    const string LegacyBitConverterInclude = "#include \"BitConverter.hpp\"";

    /// <summary>
    /// Stores the Nintendo DS-specific parser stub that avoids global std::regex initialization on ARM9.
    /// </summary>
    const string HlslShaderBindingParserStubSource = """
#ifdef DrawText
#undef DrawText
#endif
#include "HlslShaderBindingParser.hpp"

Array<::ShaderBinding*>* HlslShaderBindingParser::ParseBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy)
{
    (void)source;
    (void)bindingPolicy;
    return Array<::ShaderBinding*>::Empty();
}

Array<::ShaderBinding*>* HlslShaderBindingParser::ParseBindings(std::string source, ::ShaderBindingPolicy* bindingPolicy, List<::ShaderDefine*>* defines)
{
    (void)source;
    (void)bindingPolicy;
    (void)defines;
return Array<::ShaderBinding*>::Empty();
}
""";

    /// <summary>
    /// Stores the Nintendo DS timer include block inserted into generated core timing code.
    /// </summary>
    const string NintendoDsTimersIncludeBlock = """
extern "C" {
#include <nds/timers.h>
}

""";

    /// <summary>
    /// Stores the generated stopwatch-backed Core draw timing method that is not reliable on Nintendo DS.
    /// </summary>
    static readonly Regex CoreDrawStopwatchTimingMethodPattern = new(
        @"double\s+Core::MeasureRenderManager3DDrawMilliseconds\(\)\s*\{\s*this->DrawStopwatchValue->Restart\(\);\s*this->RenderManager3D->Draw\(\);\s*this->DrawStopwatchValue->Stop\(\);\s*return\s+this->DrawStopwatchValue->get_Elapsed\(\)\.get_TotalMilliseconds\(\);\s*\}",
        RegexOptions.CultureInvariant | RegexOptions.Singleline);

    /// <summary>
    /// Stores the Nintendo DS CPU-timer-backed Core draw timing method used by the runtime overlay.
    /// </summary>
    const string CoreNintendoDsDrawTimingMethod = """
double Core::MeasureRenderManager3DDrawMilliseconds()
{
cpuStartTiming(0);
this->RenderManager3D->Draw();
return static_cast<double>(timerTicks2usec(cpuEndTiming())) / 1000.0;}
""";

    /// <summary>
    /// Stages the generated-core tree into the Nintendo DS workspace root.
    /// </summary>
    /// <param name="sourceRootPath">Generated-core root produced by the editor.</param>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    public void Stage(string sourceRootPath, string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(sourceRootPath)) {
            throw new ArgumentException("Generated core source root must be provided.", nameof(sourceRootPath));
        } else if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string fullSourceRootPath = Path.GetFullPath(sourceRootPath);
        string fullDestinationRootPath = Path.GetFullPath(destinationRootPath);
        if (!Directory.Exists(fullSourceRootPath)) {
            throw new InvalidOperationException("Generated core source root was not found.");
        }

        if (Directory.Exists(fullDestinationRootPath)) {
            Directory.Delete(fullDestinationRootPath, recursive: true);
        }

        CopyDirectory(fullSourceRootPath, fullDestinationRootPath);
        ValidateEditorFinalizedGeneratedCore(fullDestinationRootPath);
        ApplyNintendoDsRuntimeFixups(fullDestinationRootPath);
    }

    /// <summary>
    /// Recursively copies one directory tree into the destination path.
    /// </summary>
    /// <param name="sourceDirectoryPath">Existing directory to copy.</param>
    /// <param name="destinationDirectoryPath">Destination directory to populate.</param>
    static void CopyDirectory(string sourceDirectoryPath, string destinationDirectoryPath) {
        Directory.CreateDirectory(destinationDirectoryPath);

        string[] filePaths = Directory.GetFiles(sourceDirectoryPath);
        for (int index = 0; index < filePaths.Length; index++) {
            string filePath = filePaths[index];
            string fileName = Path.GetFileName(filePath);
            string destinationFilePath = Path.Combine(destinationDirectoryPath, fileName);
            File.Copy(filePath, destinationFilePath, overwrite: true);
        }

        string[] directoryPaths = Directory.GetDirectories(sourceDirectoryPath);
        for (int index = 0; index < directoryPaths.Length; index++) {
            string directoryPath = directoryPaths[index];
            string directoryName = Path.GetFileName(directoryPath);
            string destinationChildDirectoryPath = Path.Combine(destinationDirectoryPath, directoryName);
            CopyDirectory(directoryPath, destinationChildDirectoryPath);
        }
    }

    /// <summary>
    /// Applies Nintendo DS-specific generated-core rewrites required by the native runtime.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void ApplyNintendoDsRuntimeFixups(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        NormalizePlatformConfiguration(destinationRootPath);
        ForceShadersDisabled(destinationRootPath);
        ReplaceHlslShaderBindingParserWithStub(destinationRootPath);
        RewriteCoreDrawTiming(destinationRootPath);
        EnsureHashCodeHeaderExists(destinationRootPath);
        TrimRuntimeSceneResolverHeaderIncludes(destinationRootPath);
        RemoveLegacyBitConverterIncludes(destinationRootPath);
        StripRuntimeShaderPackageDependency(destinationRootPath);
        NormalizePhysicsWorldDictionaryCompatibility(destinationRootPath);
        NormalizeRuntimeTextureOwnedAssetSeams(destinationRootPath);
        NormalizeDemoDiscLightToggleOverlayCompatibility(destinationRootPath);
    }

    /// <summary>
    /// Writes one compatibility hash helper header when the editor-generated core has not emitted it yet.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void EnsureHashCodeHeaderExists(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string hashCodeHeaderPath = Path.Combine(destinationRootPath, HashCodeHeaderRelativePath);
        if (File.Exists(hashCodeHeaderPath)) {
            return;
        }

        File.WriteAllText(hashCodeHeaderPath, HashCodeHeaderSource);
    }

    /// <summary>
    /// Verifies the staged generated-core tree came from the editor finalization pipeline instead of raw cs2.cpp output.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void ValidateEditorFinalizedGeneratedCore(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string registrationSourcePath = Path.Combine(destinationRootPath, GeneratedRuntimeComponentRegistrationSourceRelativePath);
        string registrationHeaderPath = Path.Combine(destinationRootPath, GeneratedRuntimeComponentRegistrationHeaderRelativePath);
        string registrySourcePath = Path.Combine(destinationRootPath, RuntimeComponentRegistryRelativePath);
        if (!File.Exists(registrationSourcePath) || !File.Exists(registrationHeaderPath)) {
            throw new InvalidOperationException(
                "Nintendo DS builds require the editor-finalized generated core output. Generated runtime component registration files were not found.");
        } else if (!File.Exists(registrySourcePath)) {
            throw new InvalidOperationException(
                "Nintendo DS builds require the editor-finalized generated core output. RuntimeComponentRegistry.cpp was not found.");
        }

        string registrySource = File.ReadAllText(registrySourcePath);
        if (!registrySource.Contains(GeneratedRuntimeComponentRegistrationInclude, StringComparison.Ordinal)
            || !registrySource.Contains(GeneratedRuntimeComponentRegistrationCall, StringComparison.Ordinal)) {
            throw new InvalidOperationException(
                "Nintendo DS builds require the editor-finalized generated core output. RuntimeComponentRegistry.cpp was not patched for generated runtime component registration.");
        }
    }

    /// <summary>
    /// Rewrites generated platform configuration so Nintendo DS builds do not execute Windows-host startup branches.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void NormalizePlatformConfiguration(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string configurationFilePath = Path.Combine(destinationRootPath, PlatformConfigurationRelativePath);
        if (!File.Exists(configurationFilePath)) {
            return;
        }

        string configurationSource = File.ReadAllText(configurationFilePath);
        string rewrittenConfigurationSource = configurationSource
            .Replace(WindowsHostPlatformToken, NonWindowsHostPlatformToken, StringComparison.Ordinal)
            .Replace(WindowsPlatformToken, DisabledWindowsPlatformToken, StringComparison.Ordinal)
            .Replace(EnabledShadersConfigurationToken, DisabledShadersConfigurationToken, StringComparison.Ordinal);
        if (string.Equals(configurationSource, rewrittenConfigurationSource, StringComparison.Ordinal)) {
            return;
        }

        File.WriteAllText(configurationFilePath, rewrittenConfigurationSource);
    }

    /// <summary>
    /// Replaces the runtime-only HLSL shader-binding parser with a Nintendo DS stub.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void ReplaceHlslShaderBindingParserWithStub(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string parserFilePath = Path.Combine(destinationRootPath, HlslShaderBindingParserRelativePath);
        if (!File.Exists(parserFilePath)) {
            return;
        }

        File.WriteAllText(parserFilePath, HlslShaderBindingParserStubSource);
    }

    /// <summary>
    /// Rewrites generated Core draw timing to use libnds CPU timing instead of the host Stopwatch wrapper.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void RewriteCoreDrawTiming(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, CoreSourceRelativePath),
            source => RewriteCoreDrawTimingSource(source));
    }

    /// <summary>
    /// Rewrites one generated Core source string so draw-duration diagnostics use Nintendo DS hardware timing.
    /// </summary>
    /// <param name="source">Generated Core.cpp source.</param>
    /// <returns>Generated Core.cpp source with DS draw timing seams applied.</returns>
    static string RewriteCoreDrawTimingSource(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        string rewrittenSource = CoreDrawStopwatchTimingMethodPattern.Replace(
            source,
            CoreNintendoDsDrawTimingMethod,
            count: 1);
        if (string.Equals(source, rewrittenSource, StringComparison.Ordinal)) {
            return source;
        }

        return InsertNintendoDsTimersInclude(rewrittenSource);
    }

    /// <summary>
    /// Inserts the libnds timer include into generated Core source when the timing rewrite requires it.
    /// </summary>
    /// <param name="source">Generated Core.cpp source.</param>
    /// <returns>Generated Core.cpp source containing the libnds timer include block.</returns>
    static string InsertNintendoDsTimersInclude(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        if (source.Contains("#include <nds/timers.h>", StringComparison.Ordinal)) {
            return source;
        }

        const string coreInclude = "#include \"Core.hpp\"";
        int insertionIndex = source.IndexOf(coreInclude, StringComparison.Ordinal);
        if (insertionIndex < 0) {
            return NintendoDsTimersIncludeBlock + source;
        }

        return source.Insert(insertionIndex, NintendoDsTimersIncludeBlock);
    }

    /// <summary>
    /// Forces shader support off in the generated feature manifest for Nintendo DS runtime builds.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void ForceShadersDisabled(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string featureManifestPath = Path.Combine(destinationRootPath, FeatureManifestRelativePath);
        if (!File.Exists(featureManifestPath)) {
            return;
        }

        string featureManifestSource = File.ReadAllText(featureManifestPath);
        string rewrittenFeatureManifestSource = ShaderFeatureManifestEntryPattern.Replace(
            featureManifestSource,
            match => ForcedDisabledShadersFeatureManifestToken + match.Groups["suffix"].Value,
            count: 1);
        if (string.Equals(featureManifestSource, rewrittenFeatureManifestSource, StringComparison.Ordinal)) {
            return;
        }

        File.WriteAllText(featureManifestPath, rewrittenFeatureManifestSource);
    }

    /// <summary>
    /// Removes generated resolver-header includes that create Nintendo DS-only type cycles during native compilation.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void TrimRuntimeSceneResolverHeaderIncludes(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        string resolverHeaderPath = Path.Combine(destinationRootPath, RuntimeSceneAssetReferenceResolverHeaderRelativePath);
        if (!File.Exists(resolverHeaderPath)) {
            return;
        }

        string resolverHeaderSource = File.ReadAllText(resolverHeaderPath);
        string rewrittenResolverHeaderSource = resolverHeaderSource
            .Replace("#include \"Core.hpp\"\r\n", string.Empty, StringComparison.Ordinal)
            .Replace("#include \"Core.hpp\"\n", string.Empty, StringComparison.Ordinal)
            .Replace("#include \"RenderManager3D.hpp\"\r\n", string.Empty, StringComparison.Ordinal)
            .Replace("#include \"RenderManager3D.hpp\"\n", string.Empty, StringComparison.Ordinal)
            .Replace("#include \"RenderManager2D.hpp\"\r\n", string.Empty, StringComparison.Ordinal)
            .Replace("#include \"RenderManager2D.hpp\"\n", string.Empty, StringComparison.Ordinal);
        if (string.Equals(resolverHeaderSource, rewrittenResolverHeaderSource, StringComparison.Ordinal)) {
            return;
        }

        File.WriteAllText(resolverHeaderPath, rewrittenResolverHeaderSource);
    }

    /// <summary>
    /// Removes stale generated includes that still reference the deleted top-level BitConverter header.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void RemoveLegacyBitConverterIncludes(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeAssetIdGeneratorHeaderRelativePath),
            source => RemoveExactIncludeLine(source, LegacyBitConverterInclude));
        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeAssetIdGeneratorSourceRelativePath),
            source => RemoveExactIncludeLine(source, LegacyBitConverterInclude));
    }

    /// <summary>
    /// Rewrites generated raw material resolution to the cooked-platform-owned DS material path.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void StripRuntimeShaderPackageDependency(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneAssetReferenceResolverHeaderRelativePath),
            source => RewriteRuntimeSceneResolverMaterialHeader(source));
        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneAssetReferenceResolverSourceRelativePath),
            source => RewriteRuntimeSceneResolverMaterialPath(source));
    }

    /// <summary>
    /// Adds the cooked platform-material texture resolver declaration required by Nintendo DS staged generated core.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver header source.</param>
    /// <returns>Rewritten resolver header source.</returns>
    static string RewriteRuntimeSceneResolverMaterialHeader(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        string rewrittenSource = EnsureRuntimeSceneResolverPlatformMaterialForwardDeclaration(source);

        string pathDeclaration = "void ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, std::string materialPath);";
        string assetDeclaration = "void ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::PlatformMaterialAsset* materialAsset);";
        if (!rewrittenSource.Contains(assetDeclaration, StringComparison.Ordinal)) {
            string declarationSource =
                pathDeclaration + "\n    "
                + assetDeclaration + "\n\n    ";
            rewrittenSource = rewrittenSource.Replace(
                "void ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath);",
                declarationSource + "void ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath);",
                StringComparison.Ordinal);
            if (!rewrittenSource.Contains(assetDeclaration, StringComparison.Ordinal)) {
                int classEndIndex = rewrittenSource.LastIndexOf("};", StringComparison.Ordinal);
                if (classEndIndex >= 0) {
                    rewrittenSource = rewrittenSource.Insert(
                        classEndIndex,
                        "    " + pathDeclaration + "\n    "
                        + assetDeclaration + "\n\n");
                }
            }
        }

        return rewrittenSource;
    }

    /// <summary>
    /// Ensures the generated runtime scene resolver header forward-declares cooked platform material assets.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver header source.</param>
    /// <returns>Header source with the required cooked platform-material forward declaration.</returns>
    static string EnsureRuntimeSceneResolverPlatformMaterialForwardDeclaration(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        } else if (source.Contains("class PlatformMaterialAsset;", StringComparison.Ordinal)) {
            return source;
        }

        string lineEnding = source.Contains("\r\n", StringComparison.Ordinal) ? "\r\n" : "\n";
        string rewrittenSource = source
            .Replace("class MaterialAsset;\r\n", "class MaterialAsset;\r\nclass PlatformMaterialAsset;\r\n", StringComparison.Ordinal)
            .Replace("class MaterialAsset;\n", "class MaterialAsset;\nclass PlatformMaterialAsset;\n", StringComparison.Ordinal);
        if (rewrittenSource.Contains("class PlatformMaterialAsset;", StringComparison.Ordinal)) {
            return rewrittenSource;
        }

        rewrittenSource = rewrittenSource
            .Replace("class RuntimeMaterial;\r\n", "class RuntimeMaterial;\r\nclass PlatformMaterialAsset;\r\n", StringComparison.Ordinal)
            .Replace("class RuntimeMaterial;\n", "class RuntimeMaterial;\nclass PlatformMaterialAsset;\n", StringComparison.Ordinal);
        if (rewrittenSource.Contains("class PlatformMaterialAsset;", StringComparison.Ordinal)) {
            return rewrittenSource;
        }

        int classIndex = rewrittenSource.IndexOf("class RuntimeSceneAssetReferenceResolver", StringComparison.Ordinal);
        if (classIndex >= 0) {
            return rewrittenSource.Insert(classIndex, "class PlatformMaterialAsset;" + lineEnding);
        }

        return "class PlatformMaterialAsset;" + lineEnding + rewrittenSource;
    }

    /// <summary>
    /// Rewrites one generated runtime scene resolver source string from the raw material path to the cooked-platform-owned path.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver source.</param>
    /// <returns>Rewritten resolver source.</returns>
    static string RewriteRuntimeSceneResolverMaterialPath(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        string rewrittenSource = source
            .Replace(
                "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);",
                "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);",
                string.Empty,
                StringComparison.Ordinal)
            .Replace(
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);",
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, nullptr);",
                "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);",
                StringComparison.Ordinal)
            .Replace(
                "this->ApplyMaterialDiffuseTexture(runtimeMaterial, materialAsset, fullPath);",
                "this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, materialAsset);",
                StringComparison.Ordinal);

        rewrittenSource = AddCookedMaterialTextureApplication(rewrittenSource);
        rewrittenSource = RuntimeSceneResolverMaterialReleaseGuardPattern.Replace(rewrittenSource, string.Empty, 1);
        rewrittenSource = RuntimeSceneResolverShaderReleaseGuardPattern.Replace(rewrittenSource, string.Empty, 1);
        rewrittenSource = AddRuntimeSceneResolverPlatformMaterialTextureMethod(rewrittenSource);
        return rewrittenSource;
    }

    /// <summary>
    /// Adds cooked platform-material texture binding after generated-core cooked material construction.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver source.</param>
    /// <returns>Source with cooked material texture binding inserted.</returns>
    static string AddCookedMaterialTextureApplication(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        string rewrittenSource = Regex.Replace(
            source,
            @"::RuntimeMaterial \*generatedCookedRuntimeMaterial = Core::get_Instance\(\)->get_RenderManager3D\(\)->BuildMaterialFromCooked\(generatedPlatformMaterialAsset\);\r?\n",
            "::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedPlatformMaterialAsset);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(generatedCookedRuntimeMaterial, generatedPlatformMaterialAsset);\n",
            RegexOptions.CultureInvariant);
        rewrittenSource = Regex.Replace(
            rewrittenSource,
            @"::RuntimeMaterial \*runtimeMaterial = Core::get_Instance\(\)->get_RenderManager3D\(\)->BuildMaterialFromCooked\(materialAsset\);\r?\nthis->TrackOwnedMaterial\(runtimeMaterial\);",
            "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, materialAsset);\n"
            + "this->TrackOwnedMaterial(runtimeMaterial);",
            RegexOptions.CultureInvariant);
        rewrittenSource = Regex.Replace(
            rewrittenSource,
            @"return Core::get_Instance\(\)->get_RenderManager3D\(\)->BuildMaterialFromCooked\(materialAsset\);",
            "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, materialAsset);\n"
            + "return runtimeMaterial;",
            RegexOptions.CultureInvariant);
        rewrittenSource = Regex.Replace(
            rewrittenSource,
            @"::RuntimeMaterial \*generatedCookedRuntimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromCooked\(generatedFullPath\);\r?\nthis->ActiveGeneratedMaterialsByKey->Add\(generatedAssetKey, generatedCookedRuntimeMaterial\);\r?\nthis->TrackOwnedMaterial\(generatedCookedRuntimeMaterial\);",
            "::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromCooked(generatedFullPath);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(generatedCookedRuntimeMaterial, generatedFullPath);\n"
            + "this->ActiveGeneratedMaterialsByKey->Add(generatedAssetKey, generatedCookedRuntimeMaterial);\n"
            + "this->TrackOwnedMaterial(generatedCookedRuntimeMaterial);",
            RegexOptions.CultureInvariant);
        rewrittenSource = Regex.Replace(
            rewrittenSource,
            @"::RuntimeMaterial \*runtimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromCooked\(fullPath\);\r?\nthis->TrackOwnedMaterial\(runtimeMaterial\);",
            "::RuntimeMaterial *runtimeMaterial = Core::Instance->RenderManager3D->BuildMaterialFromCooked(fullPath);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, fullPath);\n"
            + "this->TrackOwnedMaterial(runtimeMaterial);",
            RegexOptions.CultureInvariant);
        return rewrittenSource;
    }

    /// <summary>
    /// Adds a generated-core helper that binds cooked platform-material texture paths to runtime materials.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver source.</param>
    /// <returns>Source with the cooked platform-material texture helper inserted.</returns>
    static string AddRuntimeSceneResolverPlatformMaterialTextureMethod(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        } else if (source.Contains("RuntimeSceneAssetReferenceResolver::ApplyPlatformMaterialDiffuseTexture", StringComparison.Ordinal)) {
            return source;
        }

        string rewrittenSource = source;
        rewrittenSource = EnsureRuntimeSceneResolverHeaderInclude(rewrittenSource, "#include \"PlatformMaterialAsset.hpp\"", "#include \"MaterialAsset.hpp\"");

        const string insertionMarker = "void RuntimeSceneAssetReferenceResolver::ApplyMaterialDiffuseTexture";
        int insertionIndex = rewrittenSource.IndexOf(insertionMarker, StringComparison.Ordinal);
        string helperSource =
            "void RuntimeSceneAssetReferenceResolver::ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, std::string materialPath)\n"
            + "{\n"
            + "    if (String::IsNullOrWhiteSpace(materialPath))\n"
            + "    {\n"
            + "throw new ArgumentException(\"Material path must be provided.\", \"materialPath\");\n"
            + "    }\n"
            + "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(materialPath, RuntimeContentProcessorIds::MaterialAsset);\n"
            + "this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, materialAsset);\n"
            + "}\n\n"
            +
            "void RuntimeSceneAssetReferenceResolver::ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::PlatformMaterialAsset* materialAsset)\n"
            + "{\n"
            + "    if (runtimeMaterial == nullptr)\n"
            + "    {\n"
            + "throw new ArgumentNullException(\"runtimeMaterial\");\n"
            + "    }\n"
            + "    if (materialAsset == nullptr)\n"
            + "    {\n"
            + "throw new ArgumentNullException(\"materialAsset\");\n"
            + "    }\n"
            + "    if (String::IsNullOrWhiteSpace(materialAsset->TextureRelativePath))\n"
            + "    {\n"
            + "return;    }\n"
            + "std::string diffuseTexturePath = Path::Combine(this->ContentRootPath, materialAsset->TextureRelativePath);\n"
            + "::RuntimeTexture *runtimeTexture = Core::Instance->RenderManager2D->BuildTextureFromCooked(diffuseTexturePath);\n"
            + "this->TrackOwnedTexture(runtimeTexture);\n"
            + "runtimeMaterial->SetPrimaryTexture(runtimeTexture);\n"
            + "}\n\n";
        if (insertionIndex < 0) {
            return rewrittenSource + "\n" + helperSource;
        }

        return rewrittenSource.Insert(insertionIndex, helperSource);
    }

    /// <summary>
    /// Ensures the staged runtime scene resolver source contains one required include directive.
    /// </summary>
    /// <param name="source">Generated runtime scene resolver source.</param>
    /// <param name="includeDirective">Include directive that must exist.</param>
    /// <param name="anchorIncludeDirective">Existing include directive that should receive the new include immediately after it.</param>
    /// <returns>Source containing the requested include directive.</returns>
    static string EnsureRuntimeSceneResolverHeaderInclude(string source, string includeDirective, string anchorIncludeDirective) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        } else if (string.IsNullOrWhiteSpace(includeDirective)) {
            throw new ArgumentException("Include directive must be provided.", nameof(includeDirective));
        } else if (string.IsNullOrWhiteSpace(anchorIncludeDirective)) {
            throw new ArgumentException("Anchor include directive must be provided.", nameof(anchorIncludeDirective));
        } else if (source.Contains(includeDirective, StringComparison.Ordinal)) {
            return source;
        }

        string lineEnding = source.Contains("\r\n", StringComparison.Ordinal) ? "\r\n" : "\n";
        string anchoredSource = source
            .Replace(anchorIncludeDirective + "\r\n", anchorIncludeDirective + "\r\n" + includeDirective + "\r\n", StringComparison.Ordinal)
            .Replace(anchorIncludeDirective + "\n", anchorIncludeDirective + "\n" + includeDirective + "\n", StringComparison.Ordinal);
        if (!anchoredSource.Contains(includeDirective, StringComparison.Ordinal)) {
            return includeDirective + lineEnding + anchoredSource;
        }

        return anchoredSource;
    }

    /// <summary>
    /// Rewrites generated dictionary property syntax that is not supported by the Nintendo DS native dictionary shim.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void NormalizePhysicsWorldDictionaryCompatibility(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, PhysicsWorld3DSourceRelativePath),
            source => source
                .Replace(
                    "for (const auto& constraint : this->BoxBoxContactConstraintsValue->get_Values()) {\nconstraint->BeginStep();\n}",
                    "for (auto& entry : *this->BoxBoxContactConstraintsValue) {\nentry.second->BeginStep();\n}",
                    StringComparison.Ordinal)
                .Replace("entry.get_Value()", "entry.second", StringComparison.Ordinal)
                .Replace("entry.get_Key()", "entry.first", StringComparison.Ordinal));
    }

    /// <summary>
    /// Rewrites generated scene-owned asset tracking seams back to runtime-texture ownership for Nintendo DS builds.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void NormalizeRuntimeTextureOwnedAssetSeams(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneLoadResultHeaderRelativePath),
            source => source
                .Replace("List<void*>* OwnedAssets;", "List<::RuntimeTexture*>* OwnedAssets;", StringComparison.Ordinal)
                .Replace("List<void*>* get_OwnedAssets();", "List<::RuntimeTexture*>* get_OwnedAssets();", StringComparison.Ordinal)
                .Replace("RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<void*>* ownedAssets);", "RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<::RuntimeTexture*>* ownedAssets);", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneLoadResultSourceRelativePath),
            source => source
                .Replace("List<void*>* RuntimeSceneLoadResult::get_OwnedAssets()", "List<::RuntimeTexture*>* RuntimeSceneLoadResult::get_OwnedAssets()", StringComparison.Ordinal)
                .Replace("RuntimeSceneLoadResult::RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<void*>* ownedAssets)", "RuntimeSceneLoadResult::RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<::RuntimeTexture*>* ownedAssets)", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, LoadedSceneRecordHeaderRelativePath),
            source => source
                .Replace("List<void*>* OwnedAssets;", "List<::RuntimeTexture*>* OwnedAssets;", StringComparison.Ordinal)
                .Replace("List<void*>* get_OwnedAssets();", "List<::RuntimeTexture*>* get_OwnedAssets();", StringComparison.Ordinal)
                .Replace("LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<void*>* ownedAssets);", "LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<::RuntimeTexture*>* ownedAssets);", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, LoadedSceneRecordSourceRelativePath),
            source => source
                .Replace("List<void*>* LoadedSceneRecord::get_OwnedAssets()", "List<::RuntimeTexture*>* LoadedSceneRecord::get_OwnedAssets()", StringComparison.Ordinal)
                .Replace("LoadedSceneRecord::LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<void*>* ownedAssets)", "LoadedSceneRecord::LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<::RuntimeTexture*>* ownedAssets)", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneAssetReferenceResolverHeaderRelativePath),
            source => source
                .Replace("List<void*>* CompleteOwnedAssetTracking();", "List<::RuntimeTexture*>* CompleteOwnedAssetTracking();", StringComparison.Ordinal)
                .Replace("List<void*>* ActiveOwnedAssets;", "List<::RuntimeTexture*>* ActiveOwnedAssets;", StringComparison.Ordinal)
                .Replace("void TrackOwnedAsset(void* asset);", "void TrackOwnedAsset(::RuntimeTexture* asset);", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, RuntimeSceneAssetReferenceResolverSourceRelativePath),
            source => source
                .Replace("List<void*>* RuntimeSceneAssetReferenceResolver::CompleteOwnedAssetTracking()", "List<::RuntimeTexture*>* RuntimeSceneAssetReferenceResolver::CompleteOwnedAssetTracking()", StringComparison.Ordinal)
                .Replace("List<void*>* ownedAssets = new List<void*>();", "List<::RuntimeTexture*>* ownedAssets = new List<::RuntimeTexture*>();", StringComparison.Ordinal)
                .Replace("void RuntimeSceneAssetReferenceResolver::TrackOwnedAsset(void* asset)", "void RuntimeSceneAssetReferenceResolver::TrackOwnedAsset(::RuntimeTexture* asset)", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, SceneManagerHeaderRelativePath),
            source => source
                .Replace("Dictionary<void*, int32_t>* ActiveOwnedAssetReferenceCounts;", "Dictionary<::RuntimeTexture*, int32_t>* ActiveOwnedAssetReferenceCounts;", StringComparison.Ordinal)
                .Replace("void RegisterOwnedAssets(List<void*>* ownedAssets);", "void RegisterOwnedAssets(List<::RuntimeTexture*>* ownedAssets);", StringComparison.Ordinal)
                .Replace("void ReleaseOwnedAsset(void* ownedAsset);", "void ReleaseOwnedAsset(::RuntimeTexture* ownedAsset);", StringComparison.Ordinal)
                .Replace("void ReleaseOwnedAssets(List<void*>* ownedAssets);", "void ReleaseOwnedAssets(List<::RuntimeTexture*>* ownedAssets);", StringComparison.Ordinal));
        RewriteFile(
            Path.Combine(destinationRootPath, SceneManagerSourceRelativePath),
            source => {
                string rewrittenSource = source
                    .Replace("void SceneManager::RegisterOwnedAssets(List<void*>* ownedAssets)", "void SceneManager::RegisterOwnedAssets(List<::RuntimeTexture*>* ownedAssets)", StringComparison.Ordinal)
                    .Replace("void SceneManager::ReleaseOwnedAsset(void* ownedAsset)", "void SceneManager::ReleaseOwnedAsset(::RuntimeTexture* ownedAsset)", StringComparison.Ordinal)
                    .Replace("void SceneManager::ReleaseOwnedAssets(List<void*>* ownedAssets)", "void SceneManager::ReleaseOwnedAssets(List<::RuntimeTexture*>* ownedAssets)", StringComparison.Ordinal);
                rewrittenSource = Regex.Replace(
                    rewrittenSource,
                    @"const\s+void\s*\*ownedAsset",
                    "::RuntimeTexture *ownedAsset",
                    RegexOptions.CultureInvariant);
                rewrittenSource = Regex.Replace(
                    rewrittenSource,
                    @"::RuntimeTexture \*runtimeTexture = ownedAsset as RuntimeTexture;.*?throw new InvalidOperationException\(""Scene-owned runtime asset must be a runtime texture or one disposable asset\.""\);",
                    "    if (ownedAsset->get_IsDisposed())\n"
                    + "    {\n"
                    + "return;    }\n"
                    + "    if (Core::get_Instance() == nullptr || Core::get_Instance()->get_RenderManager2D() == nullptr)\n"
                    + "    {\n"
                    + "throw new InvalidOperationException(\"Runtime texture release requires an initialized 2D render manager.\");\n"
                    + "    }\n"
                    + "Core::get_Instance()->get_RenderManager2D()->ReleaseTexture(ownedAsset);\n"
                    + "ownedAsset->Dispose();",
                    RegexOptions.CultureInvariant | RegexOptions.Singleline);
                return rewrittenSource;
            });
    }

    /// <summary>
    /// Rewrites stale generated demo-disc light-toggle overlay assumptions so DS scenes can publish debug lines through the global debug overlay.
    /// </summary>
    /// <param name="destinationRootPath">Workspace-local generated-core root consumed by Docker.</param>
    static void NormalizeDemoDiscLightToggleOverlayCompatibility(string destinationRootPath) {
        if (string.IsNullOrWhiteSpace(destinationRootPath)) {
            throw new ArgumentException("Generated core destination root must be provided.", nameof(destinationRootPath));
        }

        RewriteFile(
            Path.Combine(destinationRootPath, DemoDiscLightToggleComponentSourceRelativePath),
            source => RewriteDemoDiscLightToggleOverlaySource(source));
    }

    /// <summary>
    /// Rewrites stale generated demo-disc light-toggle overlay code so Nintendo DS builds use the global debug overlay when no FPS overlay is present.
    /// </summary>
    /// <param name="source">Generated demo-disc light-toggle source.</param>
    /// <returns>Rewritten generated source with Nintendo DS overlay compatibility applied.</returns>
    static string RewriteDemoDiscLightToggleOverlaySource(string source) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        }

        bool usesCrLf = source.Contains("\r\n", StringComparison.Ordinal);
        string normalizedSource = source.Replace("\r\n", "\n", StringComparison.Ordinal);
        string rewrittenSource = normalizedSource.Replace(
            "    if (this->FpsComponentValue == nullptr && this->DebugComponentValue == nullptr)\n"
            + "    {\n"
            + "throw new InvalidOperationException(\"Light toggle component requires either an FPSComponent or DebugComponent on the same entity.\");\n"
            + "    }\n",
            string.Empty,
            StringComparison.Ordinal);
        rewrittenSource = rewrittenSource.Replace(
            "void DemoDiscLightToggleComponent::ApplyOverlayText()\n"
            + "{\n"
            + "    if (this->FpsComponentValue != nullptr)\n"
            + "    {\n"
            + "this->FpsComponentValue->set_AdditionalText(this->BuildOverlayText());\n"
            + "    }\n"
            + "    if (this->DebugComponentValue != nullptr)\n"
            + "    {\n"
            + "DebugComponent::SetAdditionalLine(DebugLightStatusLineId, this->BuildLightStatusText());\n"
            + "DebugComponent::SetAdditionalLine(DebugCameraControlsLineId, this->BuildCameraControlsText());\n"
            + "    }\n"
            + "}\n",
            "void DemoDiscLightToggleComponent::ApplyOverlayText()\n"
            + "{\n"
            + "    if (this->FpsComponentValue != nullptr)\n"
            + "    {\n"
            + "this->FpsComponentValue->set_AdditionalText(this->BuildOverlayText());\n"
            + "return;    }\n"
            + "DebugComponent::SetAdditionalLine(DebugLightStatusLineId, this->BuildLightStatusText());\n"
            + "DebugComponent::SetAdditionalLine(DebugCameraControlsLineId, this->BuildCameraControlsText());\n"
            + "}\n",
            StringComparison.Ordinal);
        rewrittenSource = rewrittenSource.Replace(
            "void DemoDiscLightToggleComponent::ClearOverlayText()\n"
            + "{\n"
            + "    if (this->DebugComponentValue == nullptr)\n"
            + "    {\n"
            + "return;    }\n"
            + "DebugComponent::ClearAdditionalLine(DebugLightStatusLineId);\n"
            + "DebugComponent::ClearAdditionalLine(DebugCameraControlsLineId);\n"
            + "}\n",
            "void DemoDiscLightToggleComponent::ClearOverlayText()\n"
            + "{\n"
            + "    if (this->FpsComponentValue != nullptr)\n"
            + "    {\n"
            + "return;    }\n"
            + "DebugComponent::ClearAdditionalLine(DebugLightStatusLineId);\n"
            + "DebugComponent::ClearAdditionalLine(DebugCameraControlsLineId);\n"
            + "}\n",
            StringComparison.Ordinal);
        if (!usesCrLf) {
            return rewrittenSource;
        }

        return rewrittenSource.Replace("\n", "\r\n", StringComparison.Ordinal);
    }

    /// <summary>
    /// Rewrites one staged generated-core file when it exists.
    /// </summary>
    /// <param name="path">Absolute staged file path to rewrite.</param>
    /// <param name="rewrite">Function that rewrites the existing file content.</param>
    static void RewriteFile(string path, Func<string, string> rewrite) {
        if (string.IsNullOrWhiteSpace(path)) {
            throw new ArgumentException("Path must be provided.", nameof(path));
        } else if (rewrite == null) {
            throw new ArgumentNullException(nameof(rewrite));
        } else if (!File.Exists(path)) {
            return;
        }

        string source = File.ReadAllText(path);
        string rewrittenSource = rewrite(source);
        if (string.Equals(source, rewrittenSource, StringComparison.Ordinal)) {
            return;
        }

        File.WriteAllText(path, rewrittenSource);
    }

    /// <summary>
    /// Removes every exact include line from one generated source string while preserving the remaining file content.
    /// </summary>
    /// <param name="source">Existing generated source.</param>
    /// <param name="includeLine">Exact include line to remove.</param>
    /// <returns>Rewritten source without the requested include line.</returns>
    static string RemoveExactIncludeLine(string source, string includeLine) {
        if (source == null) {
            throw new ArgumentNullException(nameof(source));
        } else if (string.IsNullOrWhiteSpace(includeLine)) {
            throw new ArgumentException("Include line must be provided.", nameof(includeLine));
        }

        return source
            .Replace(includeLine + "\r\n", string.Empty, StringComparison.Ordinal)
            .Replace(includeLine + "\n", string.Empty, StringComparison.Ordinal);
    }
}
