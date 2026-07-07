#ifdef DrawText
#undef DrawText
#endif
#include "PersistedComponentTypeResolver.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "runtime/array.hpp"
#include "LayoutComponent.hpp"
#include "PersistedComponentTypeResolver.hpp"
#include "float4.hpp"
#include "runtime/native_event.hpp"
#include "AnchorSpace.hpp"
#include "IAnchorBoundsProvider.hpp"
#include "int2.hpp"
#include "Entity.hpp"
#include "runtime/native_nullable.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/array.hpp"
#include "runtime/native_event.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_nullable.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

Type* PersistedComponentTypeResolver::TryResolve(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
return nullptr;    }
    if (String::Equals(componentTypeId, "helengine.AnchorComponent", StringComparison::Ordinal) || String::Equals(componentTypeId, "helengine.AnchorComponent, helengine.core", StringComparison::Ordinal))
    {
return he_cpp_type_of<LayoutComponent>("LayoutComponent");    }
Type *componentType = PersistedComponentTypeResolver::TryResolveAssemblyQualifiedType(componentTypeId);
    if (componentType != nullptr)
    {
return componentType;    }
    if (componentTypeId.Contains(static_cast<char>(','), static_cast<StringComparison>(StringComparison::Ordinal)))
    {
return nullptr;    }
Array<Assembly*> *assemblies = AppDomain::get_CurrentDomain()->GetAssemblies();
for (int32_t index = 0; index < assemblies->get_Length(); index++) {
componentType = (*assemblies)[index]->GetType(componentTypeId, false, false);
    if (componentType != nullptr)
    {
return componentType;    }
}
for (int32_t index = 0; index < CandidateAssemblyNames->get_Length(); index++) {
componentType = Type::GetType(String::Concat(componentTypeId, ", ", (*CandidateAssemblyNames)[index]), false);
    if (componentType != nullptr)
    {
return componentType;    }
}
return nullptr;}

Array<std::string>* PersistedComponentTypeResolver::CandidateAssemblyNames = new Array<std::string>({ "helengine.core", "helengine.physics", "helengine.physics3d" });

Type* PersistedComponentTypeResolver::TryResolveAssemblyQualifiedType(std::string componentTypeId)
{
    if (String::IsNullOrWhiteSpace(componentTypeId))
    {
return nullptr;    }
    if (!componentTypeId.Contains(static_cast<char>(','), static_cast<StringComparison>(StringComparison::Ordinal)))
    {
return nullptr;    }
try {
return Type::GetType(componentTypeId, false);}
catch (...) {
return nullptr;}
}

