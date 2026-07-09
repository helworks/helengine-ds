#include "platform/ds/NintendoDsPackagedAssetLoader.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <cstdio>

#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "SceneAsset.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/runtime_startup_manifest.hpp"
#include "system/io/file.hpp"

namespace helengine::ds {
    /// Normalizes one NitroFS root path to forward slashes without a trailing separator.
    /// <param name="value">Authored content-root path.</param>
    /// <returns>Normalized NitroFS content-root path.</returns>
    std::string NintendoDsPackagedAssetLoader::NormalizeRootPath(const std::string& value) {
        if (value.empty()) {
            throw ArgumentException();
        }

        std::string normalized = value;
        for (std::size_t index = 0; index < normalized.size(); index++) {
            if (normalized[index] == '\\') {
                normalized[index] = '/';
            }
        }

        while (normalized.size() > 1 && normalized.back() == '/') {
            normalized.pop_back();
        }

        return normalized;
    }

    /// Normalizes one cooked-relative path to forward slashes.
    /// <param name="value">Authored cooked-relative asset path.</param>
    /// <returns>Normalized cooked-relative asset path.</returns>
    std::string NintendoDsPackagedAssetLoader::NormalizeRelativePath(const std::string& value) {
        if (value.empty()) {
            throw ArgumentException();
        }

        std::string normalized = value;
        for (std::size_t index = 0; index < normalized.size(); index++) {
            if (normalized[index] == '\\') {
                normalized[index] = '/';
            }
        }

        if (normalized[0] == '/') {
            throw ArgumentException();
        } else if (normalized.find("..") != std::string::npos) {
            throw ArgumentException();
        }

        return normalized;
    }

    /// Creates one packaged asset loader for the supplied NitroFS content root.
    /// <param name="contentRootPath">NitroFS content root used to resolve packaged assets.</param>
    NintendoDsPackagedAssetLoader::NintendoDsPackagedAssetLoader(const std::string& contentRootPath)
        : ContentRootPath(NormalizeRootPath(contentRootPath)) {
    }

    /// Builds one full NitroFS asset path from the normalized root and cooked-relative path.
    /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
    /// <returns>Full NitroFS asset path.</returns>
    std::string NintendoDsPackagedAssetLoader::BuildContentPath(const std::string& cookedRelativePath) const {
        std::string normalizedRelativePath = NormalizeRelativePath(cookedRelativePath);
        return ContentRootPath + "/" + normalizedRelativePath;
    }

    /// Determines whether one packaged asset exists inside NitroFS.
    /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
    /// <returns>True when the packaged asset can be opened from NitroFS.</returns>
    bool NintendoDsPackagedAssetLoader::AssetExists(const std::string& cookedRelativePath) const {
        std::string fullPath = BuildContentPath(cookedRelativePath);
        FILE* file = std::fopen(fullPath.c_str(), "rb");
        if (file == nullptr) {
            return false;
        }

        std::fclose(file);
        return true;
    }

    /// Loads one packaged asset using a cooked-relative path.
    /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
    /// <returns>Deserialized packaged asset.</returns>
    Asset* NintendoDsPackagedAssetLoader::LoadAsset(const std::string& cookedRelativePath) const {
        std::string fullPath = BuildContentPath(cookedRelativePath);
        FileStream* stream = nullptr;
        try {
            stream = File::OpenRead(fullPath);
        } catch (const std::exception& exception) {
            (void)exception;
            throw FileNotFoundException();
        }

        if (stream == nullptr) {
            throw FileNotFoundException();
        }

        Asset* asset = AssetSerializer::Deserialize(stream);
        delete stream;
        return asset;
    }

    /// Determines whether the configured startup scene exists inside NitroFS.
    /// <returns>True when the generated runtime startup scene is packaged in NitroFS.</returns>
    bool NintendoDsPackagedAssetLoader::StartupSceneExists() const {
        const char* startupSceneRelativePath = he_get_runtime_startup_scene_relative_path();
        if (startupSceneRelativePath == nullptr || startupSceneRelativePath[0] == '\0') {
            return false;
        }

        return AssetExists(startupSceneRelativePath);
    }

    /// Loads the configured startup scene using the generated runtime startup manifest.
    /// <returns>Deserialized startup scene asset.</returns>
    SceneAsset* NintendoDsPackagedAssetLoader::LoadStartupScene() const {
        const char* startupSceneRelativePath = he_get_runtime_startup_scene_relative_path();
        if (startupSceneRelativePath == nullptr || startupSceneRelativePath[0] == '\0') {
            throw InvalidOperationException();
        }

        return static_cast<SceneAsset*>(LoadAsset(startupSceneRelativePath));
    }
}
#endif
