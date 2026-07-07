#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <string>

#include "IContentStreamSource.hpp"

namespace helengine::ds {
    /// Opens packaged runtime assets from the Nintendo DS NitroFS content root.
    class NintendoDsContentStreamSource : public IContentStreamSource {
    public:
        /// Creates one Nintendo DS content stream source rooted at the supplied NitroFS content path.
        /// <param name="contentRootPath">NitroFS content root used to resolve packaged asset reads.</param>
        explicit NintendoDsContentStreamSource(const std::string& contentRootPath);

        /// Opens one packaged asset stream relative to the configured NitroFS content root.
        /// <param name="assetPath">Cooked-relative runtime asset path requested by generated core.</param>
        /// <returns>Readable stream for the requested packaged asset.</returns>
        Stream* OpenRead(std::string assetPath) override;

    private:
        /// Stores the normalized NitroFS content root.
        std::string ContentRootPath;

        /// Normalizes one NitroFS content root to forward slashes without a trailing separator.
        /// <param name="value">Authored NitroFS content-root path.</param>
        /// <returns>Normalized NitroFS content-root path.</returns>
        static std::string NormalizeRootPath(const std::string& value);

        /// Normalizes one runtime asset path to forward slashes while rejecting rooted traversal.
        /// <param name="value">Runtime asset path requested by generated core.</param>
        /// <returns>Normalized relative runtime asset path.</returns>
        static std::string NormalizeRelativePath(const std::string& value);

        /// Builds one NitroFS file path for the supplied runtime asset path.
        /// <param name="assetPath">Cooked-relative runtime asset path requested by generated core.</param>
        /// <returns>Full NitroFS file path used for file reads.</returns>
        std::string BuildAssetPath(const std::string& assetPath) const;
    };
}

#endif
