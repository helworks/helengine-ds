#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class IRuntimeComponentDeserializer;
class Component;
class SceneComponentAssetRecord;
class RuntimeSceneAssetReferenceResolver;
class EngineBinaryReader;
class SceneAssetReference;

#include "IRuntimeComponentDeserializer.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"
#include "runtime/array.hpp"
#include "runtime/native_list.hpp"

class AutomaticScriptComponentRuntimeDeserializer : public ::IRuntimeComponentDeserializer
{
public:
    virtual ~AutomaticScriptComponentRuntimeDeserializer() = default;

    inline static const uint8_t CurrentVersion = 1;

    const std::string& get_ComponentTypeId();

    AutomaticScriptComponentRuntimeDeserializer(std::string componentTypeId, Type* componentType);

    ::Component* Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver);
private:
    std::string ComponentTypeIdValue;

    Type* ComponentTypeValue;

    Array<MemberInfo*>* Members;

    Array<Type*>* MemberTypes;

    static ::Component* CreateComponent(Type* componentType);

    static Type* GetMemberType(MemberInfo* memberInfo);

    static List<MemberInfo*>* GetSerializableMembers(Type* valueType);

    static bool IsSupportedMember(MemberInfo* memberInfo);

    static bool IsSupportedNestedObjectType(Type* valueType);

    static Array<Type*>* LoadMemberTypes(Array<MemberInfo*>* members);

    static Array<MemberInfo*>* LoadMembers(Type* componentType);

    static void* ReadAssetReferenceArrayValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver);

    static void* ReadDictionaryValue(::EngineBinaryReader* reader, Type* dictionaryType, Type* dictionaryKeyType, Type* dictionaryValueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver);

    static void* ReadEnumValue(::EngineBinaryReader* reader, Type* enumType, ::RuntimeSceneAssetReferenceResolver* referenceResolver);

    static void* ReadNestedObjectValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver);

    static ::SceneAssetReference* ReadOptionalReference(::EngineBinaryReader* reader);

    static void* ReadSupportedValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver);

    static void SetMemberValue(::Component* component, MemberInfo* memberInfo, void* value);

    static void SetObjectMemberValue(void* instance, MemberInfo* memberInfo, void* value);

    static bool TryReadArrayValue__out3(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver, void*& value);

    static bool TryReadEngineSerializedPayload__out2(::EngineBinaryReader* reader, Type* valueType, void*& value);

    static bool TryReadLeafValue__out2(::EngineBinaryReader* reader, Type* valueType, void*& value);
};
