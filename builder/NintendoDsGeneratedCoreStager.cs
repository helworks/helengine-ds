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
    /// Stores the generated-core relative path for the runtime feature-manifest source.
    /// </summary>
    static readonly string FeatureManifestRelativePath = Path.Combine("runtime", "feature_manifest.cpp");

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
    /// Stores the feature-manifest token that indicates shaders are enabled in generated core.
    /// </summary>
    const string EnabledShadersFeatureManifestToken = "{ HEFeature::Shaders, true";

    /// <summary>
    /// Stores the feature-manifest token that forces shaders off for Nintendo DS runtime builds.
    /// </summary>
    const string ForcedDisabledShadersFeatureManifestToken = "{ HEFeature::Shaders, false, HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }";

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
        string rewrittenFeatureManifestSource = featureManifestSource.Replace(
            EnabledShadersFeatureManifestToken,
            ForcedDisabledShadersFeatureManifestToken,
            StringComparison.Ordinal);
        if (string.Equals(featureManifestSource, rewrittenFeatureManifestSource, StringComparison.Ordinal)) {
            return;
        }

        File.WriteAllText(featureManifestPath, rewrittenFeatureManifestSource);
    }
}
