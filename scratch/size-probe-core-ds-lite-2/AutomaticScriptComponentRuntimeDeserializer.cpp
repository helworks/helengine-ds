#ifdef DrawText
#undef DrawText
#endif
#include "AutomaticScriptComponentRuntimeDeserializer.hpp"
#include "runtime/native_exceptions.hpp"
#include "Component.hpp"
#include "system/io/memory-stream.hpp"
#include "EngineBinaryReader.hpp"
#include "runtime/array.hpp"
#include "runtime/native_type.hpp"
#include "AutomaticComponentAssetReferenceSupport.hpp"
#include "SceneAssetReference.hpp"
#include "ScenePersistenceDictionaryTypeSupport.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_enum.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_list.hpp"
#include "AutomaticScriptComponentRuntimeDeserializer.hpp"
#include "EngineBinaryEndianness.hpp"
#include "ScenePersistenceIgnoreAttribute.hpp"
#include "Entity.hpp"
#include "SceneAssetReferenceFactory.hpp"
#include "EngineSerializedPayload.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "byte4.hpp"
#include "SceneEntityReference.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "system/io/stream.hpp"
#include "system/func.hpp"
#include "runtime/native_span.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "float4x4.hpp"
#include "system/action.hpp"
#include "EngineBinaryWriter.hpp"
#include "EngineBinaryHeader.hpp"
#include "system/string_comparer.hpp"
#include "system/io/memory-stream.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_enum.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/finally.hpp"
#include "runtime/native_list.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"
#include "system/string_comparer.hpp"

const std::string& AutomaticScriptComponentRuntimeDeserializer::get_ComponentTypeId()
{
return this->ComponentTypeIdValue;
}

AutomaticScriptComponentRuntimeDeserializer::AutomaticScriptComponentRuntimeDeserializer(std::string componentTypeId, Type* componentType) : ComponentTypeIdValue(), ComponentTypeValue(), Members(), MemberTypes()
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
throw ([&]() {
auto __ctor_arg_00000193 = "Component type id must be provided.";
auto __ctor_arg_00000194 = "componentTypeId";
return new ArgumentException(__ctor_arg_00000193, __ctor_arg_00000194);
})();
    }
    if (componentType == nullptr)
    {
throw new ArgumentNullException("componentType");
    }
    if (!he_cpp_type_of<Component>("Component")->IsAssignableFrom(componentType))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserializers require a ") + "Component" + std::string(" type."));
    }
this->ComponentTypeIdValue = componentTypeId;
this->ComponentTypeValue = componentType;
this->Members = AutomaticScriptComponentRuntimeDeserializer::LoadMembers(componentType);
this->MemberTypes = AutomaticScriptComponentRuntimeDeserializer::LoadMemberTypes(this->Members);
}

::Component* AutomaticScriptComponentRuntimeDeserializer::Deserialize(::SceneComponentAssetRecord* record, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (record == nullptr)
    {
throw new ArgumentNullException("record");
    }
    if (!String::Equals(record->ComponentTypeId, this->ComponentTypeIdValue, StringComparison::Ordinal))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserializer cannot deserialize '") + record->ComponentTypeId + std::string("'."));
    }
::Component *component = AutomaticScriptComponentRuntimeDeserializer::CreateComponent(this->ComponentTypeValue);
{
::MemoryStream *stream = ([&]() {
auto __ctor_arg_00000195 = ([&]() {
Array<uint8_t>* __coalesce_00000196 = record->Payload;
return __coalesce_00000196 != nullptr ? __coalesce_00000196 : Array<uint8_t>::Empty();
})();
auto __ctor_arg_00000197 = false;
return new ::MemoryStream(__ctor_arg_00000195, __ctor_arg_00000197);
})();
auto __usingDisposeGuard_00000198 = he_cpp_make_scope_exit([&]() {
if (stream != nullptr) {
stream->Dispose();
delete stream;
}
});
{
::EngineBinaryReader *reader = EngineBinaryReader::Create(stream, static_cast<EngineBinaryEndianness>(EngineBinaryEndianness::LittleEndian), true);
auto __usingDisposeGuard_00000199 = he_cpp_make_scope_exit([&]() {
if (reader != nullptr) {
reader->Dispose();
delete reader;
}
});
const uint8_t version = reader->ReadByte();
    if (version != CurrentVersion)
    {
throw new InvalidOperationException(std::string("Unsupported automatic scripted component payload version '") + std::to_string(version) + std::string("'."));
    }
const int32_t memberCount = reader->ReadInt32();
    if (memberCount != this->Members->get_Length())
    {
throw new InvalidOperationException(std::string("Packaged scripted component '") + this->ComponentTypeIdValue + std::string("' expected ") + std::to_string(this->Members->get_Length()) + std::string(" members but payload contained ") + std::to_string(memberCount) + std::string("."));
    }
for (int32_t index = 0; index < this->Members->get_Length(); index++) {
AutomaticScriptComponentRuntimeDeserializer::SetMemberValue(component, (*this->Members)[index], AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, (*this->MemberTypes)[index], referenceResolver));
}
return component;}
}
}

::Component* AutomaticScriptComponentRuntimeDeserializer::CreateComponent(Type* componentType)
{
    if (componentType == nullptr)
    {
throw new ArgumentNullException("componentType");
    }
ConstructorInfo *constructor = componentType->GetConstructor(Type::EmptyTypes);
    if (constructor == nullptr || !constructor->get_IsPublic())
    {
throw new InvalidOperationException(std::string("Scripted component type '") + componentType->get_FullName() + std::string("' must expose a public parameterless constructor."));
    }
const void *instance = Activator::CreateInstance(componentType);
    if ()
    {
throw new InvalidOperationException(std::string("Scripted component type '") + componentType->get_FullName() + std::string("' could not be instantiated."));
    }
return component;}

Type* AutomaticScriptComponentRuntimeDeserializer::GetMemberType(MemberInfo* memberInfo)
{
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
return propertyInfo->get_PropertyType();    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
return fieldInfo->get_FieldType();    }
throw new InvalidOperationException(std::string("Reflected member '") + memberInfo?.Name + std::string("' is not a supported property or field."));
}

List<MemberInfo*>* AutomaticScriptComponentRuntimeDeserializer::GetSerializableMembers(Type* valueType)
{
return new List<MemberInfo*>(Enumerable::ToArray<MemberInfo*>(Enumerable::OrderBy<MemberInfo*, std::string>(Enumerable::Where<MemberInfo*>(new List<MemberInfo*>(valueType->GetMembers(static_cast<BindingFlags>(BindingFlags::Instance | BindingFlags::Public))), new Func<TSource, bool>(static_cast<bool (*)(MemberInfo*)>(&AutomaticScriptComponentRuntimeDeserializer::IsSupportedMember))), new Func<TSource, TKey>([&](MemberInfo* member) {
return member->get_Name();
}), StringComparer::get_Ordinal())));}

bool AutomaticScriptComponentRuntimeDeserializer::IsSupportedMember(MemberInfo* memberInfo)
{
    if (memberInfo->IsDefined(he_cpp_type_of<ScenePersistenceIgnoreAttribute>("ScenePersistenceIgnoreAttribute"), false))
    {
return false;    }
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
    if (propertyInfo->get_GetMethod() == nullptr || !propertyInfo->get_GetMethod()->get_IsPublic())
    {
return false;    }
    if (propertyInfo->get_SetMethod() == nullptr || !propertyInfo->get_SetMethod()->get_IsPublic())
    {
return false;    }
    if (propertyInfo->GetIndexParameters()->get_Length() != 0)
    {
return false;    }
return true;    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
    if (!fieldInfo->get_IsPublic() || fieldInfo->get_IsStatic() || fieldInfo->get_IsInitOnly())
    {
return false;    }
return true;    }
return false;}

bool AutomaticScriptComponentRuntimeDeserializer::IsSupportedNestedObjectType(Type* valueType)
{
    if (valueType == nullptr)
    {
return false;    }
    if (valueType == he_cpp_type_of<std::string>("string") || valueType->get_IsAbstract())
    {
return false;    }
    if (!valueType->get_IsClass() && !valueType->get_IsValueType())
    {
return false;    }
    if (he_cpp_type_of<Component>("Component")->IsAssignableFrom(valueType) || he_cpp_type_of<Entity>("Entity")->IsAssignableFrom(valueType))
    {
return false;    }
    if (valueType->get_IsValueType())
    {
return true;    }
return valueType->GetConstructor(Type::EmptyTypes) != nullptr;}

Array<Type*>* AutomaticScriptComponentRuntimeDeserializer::LoadMemberTypes(Array<MemberInfo*>* members)
{
    if (members == nullptr)
    {
throw new ArgumentNullException("members");
    }
Array<Type*> *memberTypes = new Array<Type*>(members->get_Length());
for (int32_t index = 0; index < members->get_Length(); index++) {
(*memberTypes)[index] = AutomaticScriptComponentRuntimeDeserializer::GetMemberType((*members)[index]);
}
return memberTypes;}

Array<MemberInfo*>* AutomaticScriptComponentRuntimeDeserializer::LoadMembers(Type* componentType)
{
return Enumerable::ToArray<MemberInfo*>(Enumerable::OrderBy<MemberInfo*, std::string>(Enumerable::Where<MemberInfo*>(new List<MemberInfo*>(componentType->GetMembers(static_cast<BindingFlags>(BindingFlags::Instance | BindingFlags::Public))), new Func<TSource, bool>(static_cast<bool (*)(MemberInfo*)>(&AutomaticScriptComponentRuntimeDeserializer::IsSupportedMember))), new Func<TSource, TKey>([&](MemberInfo* member) {
return member->get_Name();
}), StringComparer::get_Ordinal()));}

void* AutomaticScriptComponentRuntimeDeserializer::ReadAssetReferenceArrayValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (valueType == nullptr)
    {
throw new ArgumentNullException("valueType");
    }
    if (!AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceArrayType(valueType))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support asset-backed array type '") + valueType->get_FullName() + std::string("'."));
    }
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
return nullptr;    }
    if (length < -1)
    {
throw new InvalidOperationException("Asset-reference array length cannot be negative.");
    }
Type *elementType = (valueType->GetElementType() != nullptr ? valueType->GetElementType() : throw new InvalidOperationException(std::string("Asset-reference array type '") + valueType->get_FullName() + std::string("' must expose one element type.")));
Array *resolvedValues = Array::CreateInstance(elementType, static_cast<int32_t>(length));
for (int32_t index = 0; index < length; index++) {
::SceneAssetReference *reference = AutomaticScriptComponentRuntimeDeserializer::ReadOptionalReference(reader);
    if (reference == nullptr)
    {
continue;
    }
resolvedValues->SetValue(AutomaticComponentAssetReferenceSupport::ResolveRuntimeAssetReference(elementType, reference, referenceResolver), static_cast<int32_t>(index));
}
return resolvedValues;}

void* AutomaticScriptComponentRuntimeDeserializer::ReadDictionaryValue(::EngineBinaryReader* reader, Type* dictionaryType, Type* dictionaryKeyType, Type* dictionaryValueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
else {
    if (dictionaryType == nullptr)
    {
throw new ArgumentNullException("dictionaryType");
    }
else {
    if (dictionaryKeyType == nullptr)
    {
throw new ArgumentNullException("dictionaryKeyType");
    }
else {
    if (dictionaryValueType == nullptr)
    {
throw new ArgumentNullException("dictionaryValueType");
    }
}
}
}
    if (!ScenePersistenceDictionaryTypeSupport::IsSupportedDictionaryKeyType(dictionaryKeyType))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support dictionary key type '") + dictionaryKeyType->get_FullName() + std::string("'."));
    }
const int32_t count = reader->ReadInt32();
    if (count == -1)
    {
return nullptr;    }
    if (count < -1)
    {
throw new InvalidOperationException("Dictionary entry count cannot be negative.");
    }
const void *instance = (Activator::CreateInstance(dictionaryType) != nullptr ? Activator::CreateInstance(dictionaryType) : throw new InvalidOperationException(std::string("Dictionary type '") + dictionaryType->get_FullName() + std::string("' could not be instantiated.")));
Dictionary *dictionary = he_cpp_try_cast<Dictionary>(instance);
    if (dictionary == nullptr)
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization expected one dictionary instance for '") + dictionaryType->get_FullName() + std::string("'."));
    }
for (int32_t index = 0; index < count; index++) {
const void *key = AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, dictionaryKeyType, referenceResolver);
const void *dictionaryValue = AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, dictionaryValueType, referenceResolver);
    if (key == nullptr)
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support null dictionary keys for '") + dictionaryType->get_FullName() + std::string("'."));
    }
    if (dictionary->Contains(key))
    {
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support duplicate dictionary keys for '") + dictionaryType->get_FullName() + std::string("'."));
    }
dictionary->Add(key, dictionaryValue);
}
return instance;}

void* AutomaticScriptComponentRuntimeDeserializer::ReadEnumValue(::EngineBinaryReader* reader, Type* enumType, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
Type *underlyingType = Enum::GetUnderlyingType(enumType);
const void *underlyingValue = AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, underlyingType, referenceResolver);
return Enum::ToObject(enumType, underlyingValue);}

void* AutomaticScriptComponentRuntimeDeserializer::ReadNestedObjectValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (reader->ReadByte() == 0)
    {
    if (valueType != nullptr && valueType->get_IsValueType())
    {
return Activator::CreateInstance(valueType);    }
return nullptr;    }
const void *value = (Activator::CreateInstance(valueType) != nullptr ? Activator::CreateInstance(valueType) : throw new InvalidOperationException(std::string("Nested authored object type '") + valueType->get_FullName() + std::string("' could not be instantiated.")));
List<MemberInfo*> *members = AutomaticScriptComponentRuntimeDeserializer::GetSerializableMembers(valueType);
auto __localDeleteGuard_0000019A = he_cpp_make_scope_exit([&]() {
delete members;
});
for (int32_t index = 0; index < members->get_Count(); index++) {
MemberInfo *member = (*members).get_Item(static_cast<int32_t>(index));
AutomaticScriptComponentRuntimeDeserializer::SetObjectMemberValue(value, member, AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, AutomaticScriptComponentRuntimeDeserializer::GetMemberType(member), referenceResolver));
}
return value;}

::SceneAssetReference* AutomaticScriptComponentRuntimeDeserializer::ReadOptionalReference(::EngineBinaryReader* reader)
{
return SceneAssetReferenceFactory::ReadOptionalReference(reader);}

void* AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (valueType == nullptr)
    {
throw new ArgumentNullException("valueType");
    }
    if (AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceType(valueType))
    {
::SceneAssetReference *reference = AutomaticScriptComponentRuntimeDeserializer::ReadOptionalReference(reader);
return AutomaticComponentAssetReferenceSupport::ResolveRuntimeAssetReference(valueType, reference, referenceResolver);    }
    if (AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceArrayType(valueType))
    {
return AutomaticScriptComponentRuntimeDeserializer::ReadAssetReferenceArrayValue(reader, valueType, referenceResolver);    }
void* payloadValue;
    if (AutomaticScriptComponentRuntimeDeserializer::TryReadEngineSerializedPayload__out2(reader, valueType, payloadValue))
    {
return payloadValue;    }
void* leafValue;
    if (AutomaticScriptComponentRuntimeDeserializer::TryReadLeafValue__out2(reader, valueType, leafValue))
    {
return leafValue;    }
    if (valueType->get_IsEnum())
    {
return AutomaticScriptComponentRuntimeDeserializer::ReadEnumValue(reader, valueType, referenceResolver);    }
Type* dictionaryKeyType;
Type* dictionaryValueType;
    if (ScenePersistenceDictionaryTypeSupport::IsDictionaryType__out1_out2(valueType, dictionaryKeyType, dictionaryValueType))
    {
return AutomaticScriptComponentRuntimeDeserializer::ReadDictionaryValue(reader, valueType, dictionaryKeyType, dictionaryValueType, referenceResolver);    }
void* arrayValue;
    if (AutomaticScriptComponentRuntimeDeserializer::TryReadArrayValue__out3(reader, valueType, referenceResolver, arrayValue))
    {
return arrayValue;    }
    if (AutomaticScriptComponentRuntimeDeserializer::IsSupportedNestedObjectType(valueType))
    {
return AutomaticScriptComponentRuntimeDeserializer::ReadNestedObjectValue(reader, valueType, referenceResolver);    }
throw new InvalidOperationException(std::string("Automatic scripted runtime deserialization does not support member type '") + valueType->get_FullName() + std::string("'."));
}

void AutomaticScriptComponentRuntimeDeserializer::SetMemberValue(::Component* component, MemberInfo* memberInfo, void* value)
{
    if (component == nullptr)
    {
throw new ArgumentNullException("component");
    }
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
propertyInfo->SetValue(component, value);
return;    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
fieldInfo->SetValue(component, value);
return;    }
throw new InvalidOperationException(std::string("Reflected member '") + memberInfo?.Name + std::string("' is not a supported property or field."));
}

void AutomaticScriptComponentRuntimeDeserializer::SetObjectMemberValue(void* instance, MemberInfo* memberInfo, void* value)
{
    if (instance == nullptr)
    {
throw new ArgumentNullException("instance");
    }
    PropertyInfo* propertyInfo = he_cpp_try_cast<PropertyInfo>(memberInfo);
    if (propertyInfo != nullptr)
    {
propertyInfo->SetValue(instance, value);
return;    }
    FieldInfo* fieldInfo = he_cpp_try_cast<FieldInfo>(memberInfo);
    if (fieldInfo != nullptr)
    {
fieldInfo->SetValue(instance, value);
return;    }
throw new InvalidOperationException(std::string("Reflected member '") + memberInfo?.Name + std::string("' is not a supported property or field."));
}

bool AutomaticScriptComponentRuntimeDeserializer::TryReadArrayValue__out3(::EngineBinaryReader* reader, Type* valueType, ::RuntimeSceneAssetReferenceResolver* referenceResolver, void*& value)
{
    if (!valueType->get_IsArray() || valueType->GetArrayRank() != 1)
    {
value = nullptr;
return false;    }
    if (valueType == he_cpp_type_of<Array<uint8_t>>("Array"))
    {
throw new InvalidOperationException("Automatic scripted runtime deserialization does not support raw byte[] members. Use one engine-managed binary payload type instead.");
    }
Type *elementType = (valueType->GetElementType() != nullptr ? valueType->GetElementType() : throw new InvalidOperationException(std::string("Array type '") + valueType->get_FullName() + std::string("' must expose one element type.")));
const int32_t length = reader->ReadInt32();
    if (length == -1)
    {
value = nullptr;
return true;    }
    if (length < -1)
    {
throw new InvalidOperationException("Array length cannot be negative.");
    }
Array *values = Array::CreateInstance(elementType, static_cast<int32_t>(length));
for (int32_t index = 0; index < length; index++) {
values->SetValue(AutomaticScriptComponentRuntimeDeserializer::ReadSupportedValue(reader, elementType, referenceResolver), static_cast<int32_t>(index));
}
value = values;
return true;}

bool AutomaticScriptComponentRuntimeDeserializer::TryReadEngineSerializedPayload__out2(::EngineBinaryReader* reader, Type* valueType, void*& value)
{
    if (valueType != he_cpp_type_of<EngineSerializedPayload>("EngineSerializedPayload"))
    {
value = nullptr;
return false;    }
    if (reader->ReadByte() == 0)
    {
value = nullptr;
return true;    }
const std::string formatId = reader->ReadString();
Array<uint8_t> *serializedBytes = reader->ReadByteArray();
value = EngineSerializedPayload::Restore(formatId, serializedBytes);
return true;}

bool AutomaticScriptComponentRuntimeDeserializer::TryReadLeafValue__out2(::EngineBinaryReader* reader, Type* valueType, void*& value)
{
    if (valueType == he_cpp_type_of<std::string>("string"))
    {
value = reader->ReadString();
return true;    }
    if (valueType == he_cpp_type_of<bool>("Boolean"))
    {
value = reader->ReadByte() != 0;
return true;    }
    if (valueType == he_cpp_type_of<uint8_t>("byte"))
    {
value = reader->ReadByte();
return true;    }
    if (valueType == he_cpp_type_of<uint16_t>("UInt16"))
    {
value = reader->ReadUInt16();
return true;    }
    if (valueType == he_cpp_type_of<int32_t>("Int32"))
    {
value = reader->ReadInt32();
return true;    }
    if (valueType == he_cpp_type_of<uint32_t>("UInt32"))
    {
value = reader->ReadUInt32();
return true;    }
    if (valueType == he_cpp_type_of<int64_t>("Int64"))
    {
value = reader->ReadInt64();
return true;    }
    if (valueType == he_cpp_type_of<float>("Single"))
    {
value = reader->ReadSingle();
return true;    }
    if (valueType == he_cpp_type_of<double>("double"))
    {
value = reader->ReadDouble();
return true;    }
    if (valueType == he_cpp_type_of<int2>("int2"))
    {
value = reader->ReadInt2();
return true;    }
    if (valueType == he_cpp_type_of<int4>("int4"))
    {
value = reader->ReadInt4();
return true;    }
    if (valueType == he_cpp_type_of<float2>("float2"))
    {
value = reader->ReadFloat2();
return true;    }
    if (valueType == he_cpp_type_of<float3>("float3"))
    {
value = reader->ReadFloat3();
return true;    }
    if (valueType == he_cpp_type_of<float4>("float4"))
    {
value = reader->ReadFloat4();
return true;    }
    if (valueType == he_cpp_type_of<byte4>("byte4"))
    {
value = ([&]() {
auto __ctor_arg_0000019B = static_cast<uint8_t>(reader->ReadByte());
auto __ctor_arg_0000019C = static_cast<uint8_t>(reader->ReadByte());
auto __ctor_arg_0000019D = static_cast<uint8_t>(reader->ReadByte());
auto __ctor_arg_0000019E = static_cast<uint8_t>(reader->ReadByte());
return ::byte4(__ctor_arg_0000019B, __ctor_arg_0000019C, __ctor_arg_0000019D, __ctor_arg_0000019E);
})();
return true;    }
    if (valueType == he_cpp_type_of<SceneEntityReference>("SceneEntityReference"))
    {
value = reader->ReadSceneEntityReference();
return true;    }
value = nullptr;
return false;}

