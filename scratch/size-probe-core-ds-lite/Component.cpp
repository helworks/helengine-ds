#ifdef DrawText
#undef DrawText
#endif
#include "Component.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_dictionary.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "system/string_comparer.hpp"

Component::Component() : Parent(), isDisposed(), SyntheticStringMembers(new Dictionary<std::string, std::string>(StringComparer::get_Ordinal())), SyntheticBooleanMembers(new Dictionary<std::string, bool>(StringComparer::get_Ordinal())), SyntheticInt32Members(new Dictionary<std::string, int32_t>(StringComparer::get_Ordinal())), SyntheticSingleMembers(new Dictionary<std::string, float>(StringComparer::get_Ordinal()))
{
}

::Entity* Component::get_Parent()
{
return this->Parent;
}

void Component::set_Parent(::Entity* value)
{
this->Parent = value;
}

bool Component::get_IsEditorUpdateExecutionSuppressionMarker()
{
return false;
}

::Entity* Component::get_ParentUnsafe()
{
return this->Parent;
}

bool Component::get_IsDisposed()
{
return this->isDisposed;
}

void Component::AttachToEntity(::Entity* entity)
{
    if (entity == nullptr)
    {
throw new ArgumentNullException("entity");
    }
this->ThrowIfDisposed();
this->set_Parent(entity);
}

void Component::ComponentAdded(::Entity* entity)
{
}

void Component::ComponentInitialized(::Entity* entity)
{
}

void Component::ComponentRemoved(::Entity* entity)
{
}

void Component::DetachFromEntity()
{
this->set_Parent(nullptr);
}

void Component::Dispose()
{
this->isDisposed = true;
}

bool Component::GetSyntheticBooleanMemberOrDefault(std::string memberName, bool defaultValue)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001A1 = "Synthetic member name must be provided.";
auto __ctor_arg_000001A2 = "memberName";
return new ArgumentException(__ctor_arg_000001A1, __ctor_arg_000001A2);
})();
    }
bool value;
    if (this->SyntheticBooleanMembers->TryGetValue(memberName, value))
    {
return value;    }
return defaultValue;}

int32_t Component::GetSyntheticInt32MemberOrDefault(std::string memberName, int32_t defaultValue)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001A3 = "Synthetic member name must be provided.";
auto __ctor_arg_000001A4 = "memberName";
return new ArgumentException(__ctor_arg_000001A3, __ctor_arg_000001A4);
})();
    }
int32_t value;
    if (this->SyntheticInt32Members->TryGetValue(memberName, value))
    {
return value;    }
return defaultValue;}

float Component::GetSyntheticSingleMemberOrDefault(std::string memberName, float defaultValue)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001A5 = "Synthetic member name must be provided.";
auto __ctor_arg_000001A6 = "memberName";
return new ArgumentException(__ctor_arg_000001A5, __ctor_arg_000001A6);
})();
    }
float value;
    if (this->SyntheticSingleMembers->TryGetValue(memberName, value))
    {
return value;    }
return defaultValue;}

std::string Component::GetSyntheticStringMemberOrDefault(std::string memberName, std::string defaultValue)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001A7 = "Synthetic member name must be provided.";
auto __ctor_arg_000001A8 = "memberName";
return new ArgumentException(__ctor_arg_000001A7, __ctor_arg_000001A8);
})();
    }
std::string value;
    if (this->SyntheticStringMembers->TryGetValue(memberName, value))
    {
return value;    }
return defaultValue;}

void Component::ParentEnabledChange(bool newEnabled)
{
}

void Component::ParentStaticChange(bool newEnabled)
{
}

void Component::SetSyntheticBooleanMember(std::string memberName, bool value)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001A9 = "Synthetic member name must be provided.";
auto __ctor_arg_000001AA = "memberName";
return new ArgumentException(__ctor_arg_000001A9, __ctor_arg_000001AA);
})();
    }
(*this->SyntheticBooleanMembers).get_Item(memberName) = value;
}

void Component::SetSyntheticInt32Member(std::string memberName, int32_t value)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001AB = "Synthetic member name must be provided.";
auto __ctor_arg_000001AC = "memberName";
return new ArgumentException(__ctor_arg_000001AB, __ctor_arg_000001AC);
})();
    }
(*this->SyntheticInt32Members).get_Item(memberName) = value;
}

void Component::SetSyntheticSingleMember(std::string memberName, float value)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001AD = "Synthetic member name must be provided.";
auto __ctor_arg_000001AE = "memberName";
return new ArgumentException(__ctor_arg_000001AD, __ctor_arg_000001AE);
})();
    }
(*this->SyntheticSingleMembers).get_Item(memberName) = value;
}

void Component::SetSyntheticStringMember(std::string memberName, std::string value)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_000001AF = "Synthetic member name must be provided.";
auto __ctor_arg_000001B0 = "memberName";
return new ArgumentException(__ctor_arg_000001AF, __ctor_arg_000001B0);
})();
    }
(*this->SyntheticStringMembers).get_Item(memberName) = value;
}

void Component::ThrowIfDisposed()
{
    if (this->isDisposed)
    {
throw new InvalidOperationException("Disposed components cannot be used.");
    }
}

