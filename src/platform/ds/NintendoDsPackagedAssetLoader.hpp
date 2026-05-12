#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <string>

class Asset;
class SceneAsset;

namespace helengine::ds {
    /// Loads generated-core packaged assets from the Nintendo DS NitroFS content root.
    class NintendoDsPackagedAssetLoader {
    public:
        /// Creates one packaged asset loader for the supplied NitroFS content root.
        /// <param name="contentRootPath">NitroFS content root used to resolve packaged assets.</param>
        explicit NintendoDsPackagedAssetLoader(const std::string& contentRootPath);

        /// Determines whether one packaged asset exists inside NitroFS.
        /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
        /// <returns>True when the packaged asset can be opened from NitroFS.</returns>
        bool AssetExists(const std::string& cookedRelativePath) const;

        /// Loads one packaged asset using a cooked-relative path.
        /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
        /// <returns>Deserialized packaged asset.</returns>
        Asset* LoadAsset(const std::string& cookedRelativePath) const;

        /// Determines whether the configured startup scene exists inside NitroFS.
        /// <returns>True when the generated runtime startup scene is packaged in NitroFS.</returns>
        bool StartupSceneExists() const;

        /// Loads the configured startup scene using the generated runtime startup manifest.
        /// <returns>Deserialized startup scene asset.</returns>
        SceneAsset* LoadStartupScene() const;

    private:
        /// Stores the normalized NitroFS content root.
        std::string ContentRootPath;

        /// Normalizes one NitroFS root path to forward slashes without a trailing separator.
        /// <param name="value">Authored content-root path.</param>
        /// <returns>Normalized NitroFS content-root path.</returns>
        static std::string NormalizeRootPath(const std::string& value);

        /// Normalizes one cooked-relative path to forward slashes.
        /// <param name="value">Authored cooked-relative asset path.</param>
        /// <returns>Normalized cooked-relative asset path.</returns>
        static std::string NormalizeRelativePath(const std::string& value);

        /// Builds one full NitroFS asset path from the normalized root and cooked-relative path.
        /// <param name="cookedRelativePath">Cooked-relative asset path inside NitroFS.</param>
        /// <returns>Full NitroFS asset path.</returns>
        std::string BuildContentPath(const std::string& cookedRelativePath) const;
    };
}
#endif
