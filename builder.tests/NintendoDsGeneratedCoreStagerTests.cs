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
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
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
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "feature_manifest.cpp"), "{ HEFeature::Shaders, true, HEFeatureDecisionOrigin::AutoDetected, \"Shaders\" }");
            File.WriteAllText(Path.Combine(sourceRootPath, "HlslShaderBindingParser.cpp"), "Regex HlslShaderBindingParser::BlockCommentPattern = Regex(\".*\");");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "Core.cpp"),
                "#ifdef DrawText\n"
                + "#undef DrawText\n"
                + "#endif\n"
                + "#include \"Core.hpp\"\n"
                + "double Core::MeasureRenderManager3DDrawMilliseconds()\n"
                + "{\n"
                + "this->DrawStopwatchValue->Restart();\n"
                + "this->RenderManager3D->Draw();\n"
                + "this->DrawStopwatchValue->Stop();\n"
                + "return this->DrawStopwatchValue->get_Elapsed().get_TotalMilliseconds();}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedConfigurationSource = File.ReadAllText(Path.Combine(destinationRootPath, "helcpp_config.hpp"));
            string stagedParserSource = File.ReadAllText(Path.Combine(destinationRootPath, "HlslShaderBindingParser.cpp"));
            string stagedFeatureManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "feature_manifest.cpp"));
            string stagedCoreSource = File.ReadAllText(Path.Combine(destinationRootPath, "Core.cpp"));
            Assert.Contains("#define HE_CPP_PLATFORM_WINDOWS 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("#define HE_CPP_PLATFORM_IS_WINDOWS_HOST 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("#define HE_CPP_FEATURE_SHADERS 0", stagedConfigurationSource, StringComparison.Ordinal);
            Assert.Contains("return Array<::ShaderBinding*>::Empty();", stagedParserSource);
            Assert.DoesNotContain("Regex HlslShaderBindingParser::BlockCommentPattern", stagedParserSource);
            Assert.Contains("{ HEFeature::Shaders, false, HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }", stagedFeatureManifestSource);
            Assert.Contains("#include <nds/timers.h>", stagedCoreSource, StringComparison.Ordinal);
            Assert.Contains("cpuStartTiming(0);", stagedCoreSource, StringComparison.Ordinal);
            Assert.Contains("timerTicks2usec(cpuEndTiming())", stagedCoreSource, StringComparison.Ordinal);
            Assert.DoesNotContain("DrawStopwatchValue->Restart();", stagedCoreSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies forcing shaders off rewrites the full manifest entry instead of leaving trailing source fragments behind.
    /// </summary>
    [Fact]
    public void Stage_whenFeatureManifestContainsFullShaderEntry_rewritesWholeEntryCleanly() {
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
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "runtime", "feature_manifest.cpp"),
                "#include \"feature_manifest.hpp\"\n"
                + "\n"
                + "static const HEFeatureEntry kFeatureEntries[] = {\n"
                + "    { HEFeature::Render2D, true, HEFeatureDecisionOrigin::AutoDetected, \"Render2D\" },\n"
                + "    { HEFeature::Shaders, true, HEFeatureDecisionOrigin::AutoDetected, \"Shaders\" },\n"
                + "    { HEFeature::Sprites, true, HEFeatureDecisionOrigin::AutoDetected, \"Sprites\" },\n"
                + "};\n");
            File.WriteAllText(Path.Combine(sourceRootPath, "HlslShaderBindingParser.cpp"), "Regex HlslShaderBindingParser::BlockCommentPattern = Regex(\".*\");");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedFeatureManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "feature_manifest.cpp"));
            Assert.Contains("{ HEFeature::Shaders, false, HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }", stagedFeatureManifestSource);
            Assert.DoesNotContain("HEFeatureDecisionOrigin::ForcedDisabled, \"Shaders\" }, HEFeatureDecisionOrigin::AutoDetected", stagedFeatureManifestSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager strips stale top-level BitConverter includes when generated core already carries the modern system header.
    /// </summary>
    [Fact]
    public void Stage_whenRuntimeAssetIdGeneratorIncludesLegacyBitConverterHeader_removesLegacyInclude() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "system"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeAssetIdGenerator.hpp"),
                "#pragma once\n"
                + "#include \"BitConverter.hpp\"\n"
                + "#include \"BitConverter.hpp\"\n"
                + "#include \"system/bit_converter.hpp\"\n");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeAssetIdGenerator.cpp"),
                "#include \"RuntimeAssetIdGenerator.hpp\"\n"
                + "#include \"BitConverter.hpp\"\n"
                + "#include \"system/bit_converter.hpp\"\n");
            File.WriteAllText(Path.Combine(sourceRootPath, "system", "bit_converter.hpp"), "#pragma once");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedHeaderSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeAssetIdGenerator.hpp"));
            string stagedSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeAssetIdGenerator.cpp"));
            Assert.DoesNotContain("#include \"BitConverter.hpp\"", stagedHeaderSource, StringComparison.Ordinal);
            Assert.DoesNotContain("#include \"BitConverter.hpp\"", stagedSource, StringComparison.Ordinal);
            Assert.Contains("#include \"system/bit_converter.hpp\"", stagedHeaderSource, StringComparison.Ordinal);
            Assert.Contains("#include \"system/bit_converter.hpp\"", stagedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager trims resolver-header includes that create DS-native generated-core type cycles.
    /// </summary>
    [Fact]
    public void Stage_whenResolverHeaderIncludesCoreAndRenderManager2D_trimsCycleCausingIncludes() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.hpp"),
                "#pragma once\n"
                + "#include \"Core.hpp\"\n"
                + "#include \"RenderManager3D.hpp\"\n"
                + "#include \"RenderManager2D.hpp\"\n"
                + "#include \"SceneAssetReference.hpp\"\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedResolverHeaderSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.hpp"));
            Assert.DoesNotContain("#include \"Core.hpp\"", stagedResolverHeaderSource, StringComparison.Ordinal);
            Assert.DoesNotContain("#include \"RenderManager3D.hpp\"", stagedResolverHeaderSource, StringComparison.Ordinal);
            Assert.DoesNotContain("#include \"RenderManager2D.hpp\"", stagedResolverHeaderSource, StringComparison.Ordinal);
            Assert.Contains("#include \"SceneAssetReference.hpp\"", stagedResolverHeaderSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager rewrites generated object-owned scene asset seams back to runtime-texture ownership for DS builds.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedCoreUsesObjectOwnedAssets_rewritesRuntimeTextureOwnershipSeams() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneLoadResult.hpp"),
                "class RuntimeTexture;\n"
                + "class RuntimeSceneLoadResult { List<void*>* OwnedAssets; List<void*>* get_OwnedAssets(); RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<void*>* ownedAssets); };");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneLoadResult.cpp"),
                "List<void*>* RuntimeSceneLoadResult::get_OwnedAssets() { return this->OwnedAssets; }\n"
                + "RuntimeSceneLoadResult::RuntimeSceneLoadResult(List<::Entity*>* rootEntities, List<void*>* ownedAssets) : OwnedAssets(), RootEntities() {}");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "LoadedSceneRecord.hpp"),
                "class RuntimeTexture;\n"
                + "class LoadedSceneRecord { List<void*>* OwnedAssets; List<void*>* get_OwnedAssets(); LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<void*>* ownedAssets); };");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "LoadedSceneRecord.cpp"),
                "List<void*>* LoadedSceneRecord::get_OwnedAssets() { return this->OwnedAssets; }\n"
                + "LoadedSceneRecord::LoadedSceneRecord(std::string sceneId, std::string cookedRelativePath, List<::Entity*>* rootEntities, List<void*>* ownedAssets) : CookedRelativePath(), OwnedAssets(), RootEntities(), SceneId() {}");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.hpp"),
                "class RuntimeTexture;\n"
                + "class RuntimeSceneAssetReferenceResolver { List<void*>* CompleteOwnedAssetTracking(); List<void*>* ActiveOwnedAssets; void TrackOwnedAsset(void* asset); };");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.cpp"),
                "List<void*>* RuntimeSceneAssetReferenceResolver::CompleteOwnedAssetTracking() { List<void*>* ownedAssets = new List<void*>(); return ownedAssets; }\n"
                + "void RuntimeSceneAssetReferenceResolver::TrackOwnedAsset(void* asset) { if (asset == nullptr) { return; } }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "SceneManager.hpp"),
                "class RuntimeTexture;\n"
                + "class SceneManager { Dictionary<void*, int32_t>* ActiveOwnedAssetReferenceCounts; void RegisterOwnedAssets(List<void*>* ownedAssets); void ReleaseOwnedAsset(void* ownedAsset); void ReleaseOwnedAssets(List<void*>* ownedAssets); };");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "SceneManager.cpp"),
                "void SceneManager::RegisterOwnedAssets(List<void*>* ownedAssets) { const void *ownedAsset = (*ownedAssets)[0]; }\n"
                + "void SceneManager::ReleaseOwnedAsset(void* ownedAsset) { ::RuntimeTexture *runtimeTexture = ownedAsset as RuntimeTexture; if (runtimeTexture != nullptr) { if (runtimeTexture->get_IsDisposed()) { return; } Core::get_Instance()->get_RenderManager2D()->ReleaseTexture(runtimeTexture); runtimeTexture->Dispose(); return; } IDisposable *disposable = ownedAsset as IDisposable; if (disposable != nullptr) { disposable->Dispose(); return; } throw new InvalidOperationException(\"Scene-owned runtime asset must be a runtime texture or one disposable asset.\"); }\n"
                + "void SceneManager::ReleaseOwnedAssets(List<void*>* ownedAssets) { const void *ownedAsset = (*ownedAssets)[0]; this->ReleaseOwnedAsset(ownedAsset); }");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedSceneManagerHeader = File.ReadAllText(Path.Combine(destinationRootPath, "SceneManager.hpp"));
            string stagedSceneManagerSource = File.ReadAllText(Path.Combine(destinationRootPath, "SceneManager.cpp"));
            string stagedResolverHeader = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.hpp"));
            string stagedResolverSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.cpp"));
            string stagedLoadResultHeader = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneLoadResult.hpp"));
            string stagedLoadedRecordHeader = File.ReadAllText(Path.Combine(destinationRootPath, "LoadedSceneRecord.hpp"));

            Assert.Contains("Dictionary<::RuntimeTexture*, int32_t>* ActiveOwnedAssetReferenceCounts;", stagedSceneManagerHeader, StringComparison.Ordinal);
            Assert.Contains("void RegisterOwnedAssets(List<::RuntimeTexture*>* ownedAssets);", stagedSceneManagerHeader, StringComparison.Ordinal);
            Assert.Contains("::RuntimeTexture *ownedAsset = (*ownedAssets)[0];", stagedSceneManagerSource, StringComparison.Ordinal);
            Assert.DoesNotContain("ownedAsset as RuntimeTexture", stagedSceneManagerSource, StringComparison.Ordinal);
            Assert.DoesNotContain("ownedAsset as IDisposable", stagedSceneManagerSource, StringComparison.Ordinal);
            Assert.Contains("List<::RuntimeTexture*>* CompleteOwnedAssetTracking();", stagedResolverHeader, StringComparison.Ordinal);
            Assert.Contains("void TrackOwnedAsset(::RuntimeTexture* asset);", stagedResolverHeader, StringComparison.Ordinal);
            Assert.Contains("List<::RuntimeTexture*>* ownedAssets = new List<::RuntimeTexture*>();", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("List<::RuntimeTexture*>* OwnedAssets;", stagedLoadResultHeader, StringComparison.Ordinal);
            Assert.Contains("List<::RuntimeTexture*>* OwnedAssets;", stagedLoadedRecordHeader, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager rewrites generated shader-backed material loading to the cooked-platform-owned DS runtime path.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedMaterialResolverLoadsShaderPackages_rewritesToCookedPlatformOwnedMaterialPath() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.cpp"),
                "::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveMaterial(::SceneAssetReference* reference)\n"
                + "{\n"
                + "const std::string fullPath = this->ResolveFileBackedAssetPath(reference);\n"
                + "::MaterialAsset *materialAsset = this->AssetContentManager->Load<MaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n"
                + "::ShaderAsset *shaderAsset = this->AssetContentManager->Load<ShaderAsset*>(this->ResolveShaderPackagePath(materialAsset->ShaderAssetId), RuntimeContentProcessorIds::ShaderAsset);\n"
                + "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromRaw(materialAsset, shaderAsset);\n"
                + "return runtimeMaterial;\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedResolverSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.cpp"));
            Assert.DoesNotContain("Load<ShaderAsset*>", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("Load<PlatformMaterialAsset*>", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("BuildMaterialFromCooked(materialAsset)", stagedResolverSource, StringComparison.Ordinal);
            Assert.DoesNotContain("BuildMaterialFromRaw(materialAsset, nullptr)", stagedResolverSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies cooked Nintendo DS materials still apply their cooked diffuse texture path after the resolver is rewritten away from raw material assets.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedMaterialResolverUsesCookedPlatformMaterial_keepsCookedTextureApplication() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.hpp"),
                "class RuntimeSceneAssetReferenceResolver\n"
                + "{\n"
                + "void ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath);\n"
                + "};\n");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.cpp"),
                "::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveMaterial(::SceneAssetReference* reference)\n"
                + "{\n"
                + "const std::string fullPath = this->ResolveFileBackedAssetPath(reference);\n"
                + "::PlatformMaterialAsset *materialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(fullPath, RuntimeContentProcessorIds::MaterialAsset);\n"
                + "::RuntimeMaterial *runtimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(materialAsset);\n"
                + "this->TrackOwnedMaterial(runtimeMaterial);\n"
                + "return runtimeMaterial;\n"
                + "}\n"
                + "::RuntimeMaterial* RuntimeSceneAssetReferenceResolver::ResolveGeneratedMaterial(::SceneAssetReference* reference)\n"
                + "{\n"
                + "const std::string generatedFullPath = this->ResolveFileBackedAssetPath(reference);\n"
                + "::PlatformMaterialAsset *generatedPlatformMaterialAsset = this->AssetContentManager->Load<PlatformMaterialAsset*>(generatedFullPath, RuntimeContentProcessorIds::MaterialAsset);\n"
                + "::RuntimeMaterial *generatedCookedRuntimeMaterial = Core::get_Instance()->get_RenderManager3D()->BuildMaterialFromCooked(generatedPlatformMaterialAsset);\n"
                + "return generatedCookedRuntimeMaterial;\n"
                + "}\n"
                + "void RuntimeSceneAssetReferenceResolver::ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath)\n"
                + "{\n"
                + "if (String::IsNullOrWhiteSpace(materialAsset->DiffuseTextureAssetId)) { return; }\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedResolverHeader = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.hpp"));
            string stagedResolverSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.cpp"));
            Assert.Contains("ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::PlatformMaterialAsset* materialAsset)", stagedResolverHeader, StringComparison.Ordinal);
            Assert.Contains("this->ApplyPlatformMaterialDiffuseTexture(runtimeMaterial, materialAsset);\nthis->TrackOwnedMaterial(runtimeMaterial);", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("this->ApplyPlatformMaterialDiffuseTexture(generatedCookedRuntimeMaterial, generatedPlatformMaterialAsset);", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("materialAsset->TextureRelativePath", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("TrackOwnedTexture(runtimeTexture)", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("#include \"ShaderRuntimeMaterial.hpp\"", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("#include \"StandardMaterialTextureBindingDefaults.hpp\"", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("::ShaderRuntimeMaterial *shaderRuntimeMaterial = dynamic_cast<ShaderRuntimeMaterial*>(runtimeMaterial);", stagedResolverSource, StringComparison.Ordinal);
            Assert.Contains("shaderRuntimeMaterial->get_Properties()->SetTexture(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, runtimeTexture);", stagedResolverSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the staged runtime startup manifest preserves the editor-authored startup scene payload path.
    /// </summary>
    [Fact]
    public void Stage_whenRuntimeStartupManifestTargetsAnotherScene_preservesAuthoredStartupScenePath() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"),
                "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/scenes/rendering/colored_cube_grid.hasset\"; }");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp"));
            Assert.Contains("cooked/scenes/rendering/colored_cube_grid.hasset", stagedManifestSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the staged runtime startup manifest preserves the current constant-backed startup-scene format emitted by editor finalization.
    /// </summary>
    [Fact]
    public void Stage_whenRuntimeStartupManifestUsesConstantBackedScenePath_preservesAuthoredStartupScenePath() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"),
                "#include \"runtime/runtime_startup_manifest.hpp\"\n"
                + "\n"
                + "static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/rendering/colored_cube_grid.hasset\";\n"
                + "static const char kRuntimePlatformName[] = \"ds\";\n"
                + "static const char kRuntimePlatformVersion[] = \"1.0.0\";\n"
                + "\n"
                + "const char* he_get_runtime_startup_scene_relative_path() {\n"
                + "    return kRuntimeStartupSceneRelativePath;\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp"));
            Assert.Contains("static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/rendering/colored_cube_grid.hasset\";", stagedManifestSource, StringComparison.Ordinal);
            Assert.Contains("return kRuntimeStartupSceneRelativePath;", stagedManifestSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager rejects raw generated-core output that was not finalized by the editor regeneration pipeline.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedCoreWasNotEditorFinalized_throws_clear_error() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { return new ::RuntimeComponentRegistry(); }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");

            NintendoDsGeneratedCoreStager stager = new();
            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => stager.Stage(sourceRootPath, destinationRootPath));
            Assert.Contains("editor-finalized generated core output", exception.Message, StringComparison.Ordinal);
            Assert.Contains("Generated runtime component registration files were not found", exception.Message, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager declares the cooked platform-material texture helper even when the generated resolver header no longer exposes the legacy raw-material helper declaration.
    /// </summary>
    [Fact]
    public void Stage_whenResolverHeaderOmitsLegacyMaterialHelper_stillDeclaresPlatformMaterialHelper() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.hpp"),
                "#pragma once\n"
                + "class MaterialAsset;\n"
                + "class RuntimeMaterial;\n"
                + "class RuntimeSceneAssetReferenceResolver {\n"
                + "private:\n"
                + "    bool TryResolveSourceTexturePath(std::string materialPath, std::string assetId, std::string& texturePath);\n"
                + "};\n");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.cpp"),
                "#include \"MaterialAsset.hpp\"\n"
                + "void RuntimeSceneAssetReferenceResolver::ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath)\n"
                + "{\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedResolverHeaderSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.hpp"));
            Assert.Contains("class PlatformMaterialAsset;", stagedResolverHeaderSource, StringComparison.Ordinal);
            Assert.Contains("void ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::PlatformMaterialAsset* materialAsset);", stagedResolverHeaderSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager still forward-declares cooked platform materials when the generated resolver header no longer forward-declares raw material assets.
    /// </summary>
    [Fact]
    public void Stage_whenResolverHeaderOmitsMaterialAssetForwardDeclaration_stillDeclaresPlatformMaterialAsset() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.hpp"),
                "#pragma once\n"
                + "class RuntimeMaterial;\n"
                + "class RuntimeSceneAssetReferenceResolver {\n"
                + "private:\n"
                + "    bool TryResolveSourceTexturePath(std::string materialPath, std::string assetId, std::string& texturePath);\n"
                + "};\n");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeSceneAssetReferenceResolver.cpp"),
                "#include \"MaterialAsset.hpp\"\n"
                + "void RuntimeSceneAssetReferenceResolver::ApplyMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::MaterialAsset* materialAsset, std::string materialPath)\n"
                + "{\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedResolverHeaderSource = File.ReadAllText(Path.Combine(destinationRootPath, "RuntimeSceneAssetReferenceResolver.hpp"));
            Assert.Contains("class PlatformMaterialAsset;", stagedResolverHeaderSource, StringComparison.Ordinal);
            Assert.Contains("void ApplyPlatformMaterialDiffuseTexture(::RuntimeMaterial* runtimeMaterial, ::PlatformMaterialAsset* materialAsset);", stagedResolverHeaderSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager rewrites generated physics dictionary property syntax into the native dictionary entry access supported by Nintendo DS builds.
    /// </summary>
    [Fact]
    public void Stage_whenPhysicsWorldUsesDictionaryPropertySyntax_rewritesEntryAccessForNativeDictionary() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "PhysicsWorld3D.cpp"),
                "void PhysicsWorld3D::PrepareBoxBoxContactConstraints()\n"
                + "{\n"
                + "for (const auto& constraint : this->BoxBoxContactConstraintsValue->get_Values()) {\n"
                + "constraint->BeginStep();\n"
                + "}\n"
                + "}\n"
                + "\n"
                + "void PhysicsWorld3D::PruneStaleBoxBoxContactConstraints()\n"
                + "{\n"
                + "for (const auto& entry : *this->BoxBoxContactConstraintsValue) {\n"
                + "    if (!entry.get_Value()->get_WasTouchedThisStep())\n"
                + "    {\n"
                + "StaleBoxBoxContactConstraintKeysValue->Add(entry.get_Key());\n"
                + "    }\n"
                + "}\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedPhysicsWorldSource = File.ReadAllText(Path.Combine(destinationRootPath, "PhysicsWorld3D.cpp"));
            Assert.Contains("for (auto& entry : *this->BoxBoxContactConstraintsValue) {", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.Contains("entry.second->BeginStep();", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.Contains("entry.second->get_WasTouchedThisStep()", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.Contains("StaleBoxBoxContactConstraintKeysValue->Add(entry.first);", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.DoesNotContain("get_Values()", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.DoesNotContain("entry.get_Key()", stagedPhysicsWorldSource, StringComparison.Ordinal);
            Assert.DoesNotContain("entry.get_Value()", stagedPhysicsWorldSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the stager relaxes stale same-entity debug-overlay assumptions in generated demo-disc light-toggle code.
    /// </summary>
    [Fact]
    public void Stage_whenGeneratedDemoDiscLightToggleRequiresSameEntityDebugOverlay_rewritesToGlobalDebugOverlayPath() {
        string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-generated-core-" + Guid.NewGuid().ToString("N"));
        string sourceRootPath = Path.Combine(rootPath, "source");
        string destinationRootPath = Path.Combine(rootPath, "workspace", "ds", "generated-core");

        try {
            Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
            File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "helengine_core_amalgamated.cpp"), "int helengine_core_fixture = 1;");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
            File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"),
                "#include \"RuntimeComponentRegistry.hpp\"\n"
                + "#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\n"
                + "::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
            File.WriteAllText(Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \"cooked/startup.hasset\"; }");
            File.WriteAllText(
                Path.Combine(sourceRootPath, "DemoDiscLightToggleComponent.cpp"),
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
                + "}\n"
                + "void DemoDiscLightToggleComponent::BindOverlayOwner(Entity* entity)\n"
                + "{\n"
                + "    if (entity->get_Components() == nullptr)\n"
                + "    {\n"
                + "throw new InvalidOperationException(\"Light toggle component requires an initialized overlay owner.\");\n"
                + "    }\n"
                + "    if (this->FpsComponentValue == nullptr && this->DebugComponentValue == nullptr)\n"
                + "    {\n"
                + "throw new InvalidOperationException(\"Light toggle component requires either an FPSComponent or DebugComponent on the same entity.\");\n"
                + "    }\n"
                + "}\n"
                + "void DemoDiscLightToggleComponent::ClearOverlayText()\n"
                + "{\n"
                + "    if (this->DebugComponentValue == nullptr)\n"
                + "    {\n"
                + "return;    }\n"
                + "DebugComponent::ClearAdditionalLine(DebugLightStatusLineId);\n"
                + "DebugComponent::ClearAdditionalLine(DebugCameraControlsLineId);\n"
                + "}\n");

            NintendoDsGeneratedCoreStager stager = new();
            stager.Stage(sourceRootPath, destinationRootPath);

            string stagedSource = File.ReadAllText(Path.Combine(destinationRootPath, "DemoDiscLightToggleComponent.cpp"));
            Assert.DoesNotContain("Light toggle component requires either an FPSComponent or DebugComponent on the same entity.", stagedSource, StringComparison.Ordinal);
            Assert.Contains("DebugComponent::SetAdditionalLine(DebugLightStatusLineId, this->BuildLightStatusText());", stagedSource, StringComparison.Ordinal);
            Assert.Contains("DebugComponent::SetAdditionalLine(DebugCameraControlsLineId, this->BuildCameraControlsText());", stagedSource, StringComparison.Ordinal);
            Assert.Contains("if (this->FpsComponentValue != nullptr)", stagedSource, StringComparison.Ordinal);
            Assert.Contains("DebugComponent::ClearAdditionalLine(DebugLightStatusLineId);", stagedSource, StringComparison.Ordinal);
            Assert.Contains("DebugComponent::ClearAdditionalLine(DebugCameraControlsLineId);", stagedSource, StringComparison.Ordinal);
            Assert.DoesNotContain("if (this->DebugComponentValue == nullptr)", stagedSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }
}
