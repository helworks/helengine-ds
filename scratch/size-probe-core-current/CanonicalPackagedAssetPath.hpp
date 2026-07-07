#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class CanonicalPackagedAssetPath
{
public:
    virtual ~CanonicalPackagedAssetPath() = default;

    static std::string Normalize(std::string path);

    static std::string ValidateCanonical(std::string path);
private:
    static std::string CollapseAndValidateSegments(std::string normalizedPath, std::string originalPath);

    static bool IsRootedPath(std::string path);

    static std::string NormalizeSlashAndCase(std::string path);
};
