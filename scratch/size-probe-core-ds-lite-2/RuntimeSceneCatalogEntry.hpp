#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"

class RuntimeSceneCatalogEntry
{
public:
    virtual ~RuntimeSceneCatalogEntry() = default;

    std::string SceneId;

    const std::string& get_SceneId();

    std::string CookedRelativePath;

    const std::string& get_CookedRelativePath();

    RuntimeSceneCatalogEntry(std::string sceneId, std::string cookedRelativePath);
private:
    static bool IsRootedRuntimePath(std::string path);

    static std::string NormalizeCookedRelativePath(std::string cookedRelativePath);
};
