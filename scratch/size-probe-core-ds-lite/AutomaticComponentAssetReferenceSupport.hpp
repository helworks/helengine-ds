#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class SceneAssetReference;
class RuntimeSceneAssetReferenceResolver;

#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

class AutomaticComponentAssetReferenceSupport
{
public:
    virtual ~AutomaticComponentAssetReferenceSupport() = default;

    static std::string BuildIndexedReferenceName(std::string memberName, int32_t index);

    static std::string BuildReferenceName(std::string memberName);

    static bool IsSupportedAssetReferenceArrayType(Type* valueType);

    static bool IsSupportedAssetReferenceType(Type* valueType);

    static void* ResolveRuntimeAssetReference(Type* valueType, ::SceneAssetReference* reference, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    inline static const std::string UnsupportedAssetReferenceTypeMessage = "Automatic component asset-reference support does not handle the supplied member type.";
};
