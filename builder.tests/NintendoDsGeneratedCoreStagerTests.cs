namespace helengine.ds.builder.tests;

/// <summary>
/// Verifies the Nintendo DS generated-core staging behavior used before native packaging.
/// </summary>
public class NintendoDsGeneratedCoreStagerTests {
    /// <summary>
    /// Verifies the stager copies the generated-core tree into the Nintendo DS workspace.
    /// </summary>
    [Fact]
    public void Stage_copies_generated_core_tree_into_workspace() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            Assert.True(File.Exists(Path.Combine(destinationRootPath, "helcpp_config.hpp")));
            Assert.True(File.Exists(Path.Combine(destinationRootPath, "helengine_core_amalgamated.cpp")));
            Assert.True(File.Exists(Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp")));
            Assert.Equal(
                "int helengine_core_fixture = 1;",
                File.ReadAllText(Path.Combine(destinationRootPath, "helengine_core_amalgamated.cpp")));
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager rewrites Nintendo DS runtime compatibility seams in generated core before native compilation.
    /// </summary>
    [Fact]
    public void Stage_rewrites_nintendo_ds_runtime_compatibility_seams() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(
                Path.Combine(sourceRootPath, "helcpp_config.hpp"),
                "#pragma once\n"
                + "#define HE_CPP_PLATFORM_WINDOWS 1\n"
                + "#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 1\n"
                + "#define HE_CPP_FEATURE_SHADERS 1\n");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "feature_manifest.cpp"), "{ HEFeature::Shaders, true, HEFeatureDecisionOrigin::AutoDetected, \"Shaders\" }");
            File.WriteAllText(Path.Combine(sourceRootPath, "HlslShaderBindingParser.cpp"), "Regex HlslShaderBindingParser::BlockCommentPattern = Regex(\".*\");");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedConfigurationSource = File.ReadAllText(Path.Combine(destinationRootPath, "helcpp_config.hpp"));
            string stagedParserSource = File.ReadAllText(Path.Combine(destinationRootPath, "HlslShaderBindingParser.cpp"));
            string stagedFeatureManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "feature_manifest.cpp"));
            Assert.Contains("#define HE_CPP_PLATFORM_WINDOWS 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("#define HE_CPP_FEATURE_SHADERS 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("return Array<::ShaderBinding*>::Empty();", stagedParserSource);
            Assert.DoesNotContain("Regex HlslShaderBindingParser::BlockCommentPattern", stagedParserSource);
            Assert.Contains("{ HEFeature::Shaders, false, HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }", stagedFeatureManifestSource);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }
}
