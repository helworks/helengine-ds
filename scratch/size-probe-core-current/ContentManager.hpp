#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class ContentProcessorRegistration;
template <typename T> class IContentProcessor_1;

#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"

class ContentManager
{
public:
    virtual ~ContentManager() = default;

    const std::string& get_RootDirectory();

    ContentManager(std::string rootDirectory);

    bool IsProcessorRegistered(std::string processorId);

    template <typename T>
    T Load(std::string assetPath, std::string processorId);

    template <typename T>
    T Load(std::string assetPath, ::IContentProcessor_1<T>* processor);

    void RegisterProcessor(::ContentProcessorRegistration* registration);

    template <typename T>
    void RegisterProcessor(std::string processorId, ::IContentProcessor_1<T>* processor, Array<std::string>* extensions);
private:
    inline static const std::string TextContentProcessorId = "core.text-content";

    inline static const std::string RawByteContentProcessorId = "core.raw-byte-content";

    inline static const std::string WildcardExtension = "*";

    std::string RootDirectoryPath;

    Dictionary<std::string, ::ContentProcessorRegistration*>* ProcessorRegistrationsById;

    Dictionary<Type*, Dictionary<std::string, ::ContentProcessorRegistration*>*>* DefaultProcessorsByTypeAndExtension;

    static std::string CombineVirtualRootedPath(std::string rootPath, std::string relativePath);

    static std::string EnsureTrailingDirectorySeparator(std::string path);

    Dictionary<std::string, ::ContentProcessorRegistration*>* GetOrCreateTypeRegistrationMap(Type* outputType);

    static bool HasVirtualRootPrefix(std::string path);

    template <typename T>
    T LoadProcessedContent(std::string fullPath, ::IContentProcessor_1<T>* processor);

    std::string NormalizeExtension(std::string extension);

    void RegisterBuiltInProcessors();

    std::string ResolveContentPath(std::string assetPath);

    ::ContentProcessorRegistration* ResolveDefaultProcessorRegistration(Type* requestedType, std::string fullPath);

    ::ContentProcessorRegistration* ResolveExplicitProcessorRegistration(Type* requestedType, std::string processorId);

    template <typename T>
    ::IContentProcessor_1<T>* ResolveProcessor(std::string fullPath, std::string processorId);

    static std::string TrimLeadingDirectorySeparators(std::string path);

    bool TryResolveRegisteredExtension__out2(std::string fileName, Dictionary<std::string, ::ContentProcessorRegistration*>* registrationsByExtension, std::string& matchedExtension);
};
