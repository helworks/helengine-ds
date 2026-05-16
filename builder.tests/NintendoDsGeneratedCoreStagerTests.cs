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
    /// Verifies the staged runtime startup manifest rewrites Nintendo DS startup ownership to the demo-disc main menu payload.
    /// </summary>
    [Fact]
    public void Stage_whenRuntimeStartupManifestTargetsAnotherScene_rewritesToDemoDiscMainMenuDs() {
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
            Assert.Contains("cooked/scenes/DemoDiscMainMenuDs.hasset", stagedManifestSource, StringComparison.Ordinal);
        } finally {
            if (Directory.Exists(rootPath)) {
                Directory.Delete(rootPath, recursive: true);
            }
        }
    }

    /// <summary>
    /// Verifies the staged runtime startup manifest rewrites the current constant-backed startup-scene format emitted by editor finalization.
    /// </summary>
    [Fact]
    public void Stage_whenRuntimeStartupManifestUsesConstantBackedScenePath_rewritesToDemoDiscMainMenuDs() {
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
            Assert.Contains("static const char kRuntimeStartupSceneRelativePath[] = \"cooked/scenes/DemoDiscMainMenuDs.hasset\";", stagedManifestSource, StringComparison.Ordinal);
            Assert.Contains("return kRuntimeStartupSceneRelativePath;", stagedManifestSource, StringComparison.Ordinal);
            Assert.DoesNotContain("cooked/scenes/rendering/colored_cube_grid.hasset", stagedManifestSource, StringComparison.Ordinal);
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
}
