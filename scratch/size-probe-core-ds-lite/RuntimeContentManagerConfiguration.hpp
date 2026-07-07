#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ContentManager;
template <typename T> class IContentProcessor_1;

#include "runtime/native_string.hpp"
#include "runtime/array.hpp"

class RuntimeContentManagerConfiguration
{
public:
    virtual ~RuntimeContentManagerConfiguration() = default;

    static void ConfigureSharedAssetContentManager(::ContentManager* contentManager);
private:
    inline static const std::string MaterialAssetExtension = ".hasset";

    inline static const std::string FontAssetExtension = ".hefont";

    template <typename T>
    static void RegisterProcessorIfMissing(::ContentManager* contentManager, std::string processorId, ::IContentProcessor_1<T>* processor, Array<std::string>* extensions);
};
