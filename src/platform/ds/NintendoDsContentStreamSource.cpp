#include "platform/ds/NintendoDsContentStreamSource.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include <stdexcept>

#include "system/io/file.hpp"

namespace helengine::ds {
    /// Normalizes one NitroFS content root to forward slashes without a trailing separator.
    /// <param name="value">Authored NitroFS content-root path.</param>
    /// <returns>Normalized NitroFS content-root path.</returns>
    std::string NintendoDsContentStreamSource::NormalizeRootPath(const std::string& value) {
        if (value.empty()) {
            throw std::invalid_argument("Nintendo DS content root path is required.");
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

    /// Normalizes one runtime asset path to forward slashes while rejecting rooted traversal.
    /// <param name="value">Runtime asset path requested by generated core.</param>
    /// <returns>Normalized relative runtime asset path.</returns>
    std::string NintendoDsContentStreamSource::NormalizeRelativePath(const std::string& value) {
        if (value.empty()) {
            throw std::invalid_argument("Nintendo DS runtime asset path is required.");
        }

        std::string normalized = value;
        for (std::size_t index = 0; index < normalized.size(); index++) {
            if (normalized[index] == '\\') {
                normalized[index] = '/';
            }
        }

        if (normalized[0] == '/' || normalized.find("..") != std::string::npos) {
            throw std::invalid_argument("Nintendo DS runtime asset path must stay inside NitroFS.");
        }

        return normalized;
    }

    /// Creates one Nintendo DS content stream source rooted at the supplied NitroFS content path.
    /// <param name="contentRootPath">NitroFS content root used to resolve packaged asset reads.</param>
    NintendoDsContentStreamSource::NintendoDsContentStreamSource(const std::string& contentRootPath)
        : ContentRootPath(NormalizeRootPath(contentRootPath)) {
    }

    /// Builds one NitroFS file path for the supplied runtime asset path.
    /// <param name="assetPath">Cooked-relative runtime asset path requested by generated core.</param>
    /// <returns>Full NitroFS file path used for file reads.</returns>
    std::string NintendoDsContentStreamSource::BuildAssetPath(const std::string& assetPath) const {
        const std::string normalizedRelativePath = NormalizeRelativePath(assetPath);
        return ContentRootPath + "/" + normalizedRelativePath;
    }

    /// Opens one packaged asset stream relative to the configured NitroFS content root.
    /// <param name="assetPath">Cooked-relative runtime asset path requested by generated core.</param>
    /// <returns>Readable stream for the requested packaged asset.</returns>
    Stream* NintendoDsContentStreamSource::OpenRead(std::string assetPath) {
        return File::OpenRead(BuildAssetPath(assetPath));
    }
}

#endif
