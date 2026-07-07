#ifdef DrawText
#undef DrawText
#endif
#include "ScenePersistenceDictionaryTypeSupport.hpp"
#include "runtime/array.hpp"
#include "runtime/native_type.hpp"
#include "ScenePersistenceDictionaryTypeSupport.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_enum.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/array.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_enum.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

int32_t ScenePersistenceDictionaryTypeSupport::CompareKeys(void* leftKey, void* rightKey, Type* keyType)
{
    if (keyType == nullptr)
    {
throw new ArgumentNullException("keyType");
    }
    if (!ScenePersistenceDictionaryTypeSupport::IsSupportedDictionaryKeyType(keyType))
    {
throw new InvalidOperationException(std::string("Dictionary key type '") + keyType->get_FullName() + std::string("' is not supported by shared scene persistence."));
    }
    if (leftKey == nullptr || rightKey == nullptr)
    {
throw new InvalidOperationException("Shared scene persistence dictionary keys cannot be null.");
    }
    if (keyType == he_cpp_type_of<std::string>("string"))
    {
return String::CompareOrdinal((*static_cast<std::string*>(leftKey)), (*static_cast<std::string*>(rightKey)));    }
    IComparable* comparableKey = he_cpp_try_cast<IComparable>(leftKey);
    if (comparableKey != nullptr)
    {
return comparableKey->CompareTo(rightKey);    }
throw new InvalidOperationException(std::string("Dictionary key type '") + keyType->get_FullName() + std::string("' does not expose one supported deterministic comparer."));
}

bool ScenePersistenceDictionaryTypeSupport::IsDictionaryType__out1_out2(Type* valueType, Type*& keyType, Type*& dictionaryValueType)
{
    if (valueType != nullptr && valueType->get_IsGenericType() && valueType->GetGenericTypeDefinition() == he_cpp_type_of<Dictionary<TKey*, TValue*>>("Dictionary"))
    {
Array<Type*> *genericArguments = valueType->GetGenericArguments();
keyType = (*genericArguments)[0];
dictionaryValueType = (*genericArguments)[1];
return true;    }
keyType = nullptr;
dictionaryValueType = nullptr;
return false;}

bool ScenePersistenceDictionaryTypeSupport::IsSupportedDictionaryKeyType(Type* keyType)
{
    if (keyType == nullptr)
    {
return false;    }
    if (SupportedScalarKeyTypes->Contains(keyType))
    {
return true;    }
    if (!keyType->get_IsEnum())
    {
return false;    }
return SupportedScalarKeyTypes->Contains(Enum::GetUnderlyingType(keyType));}

HashSet<Type>* ScenePersistenceDictionaryTypeSupport::SupportedScalarKeyTypes = new HashSet<Type>();

