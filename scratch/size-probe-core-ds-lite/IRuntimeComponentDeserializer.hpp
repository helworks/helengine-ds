#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Component;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;

#include "runtime/native_string.hpp"

class IRuntimeComponentDeserializer
{
public:
    virtual const std::string& get_ComponentTypeId() = 0;

    virtual ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver) = 0;
};
