#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class Entity;

#include "runtime/native_disposable.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"
#include "runtime/native_dictionary.hpp"

class Component : public ::IDisposable
{
public:
    virtual ~Component() = default;

    Component();

    ::Entity* Parent;

    ::Entity* get_Parent();
    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();

    void AttachToEntity(::Entity* entity);

    virtual void ComponentAdded(::Entity* entity);

    virtual void ComponentInitialized(::Entity* entity);

    virtual void ComponentRemoved(::Entity* entity);

    void DetachFromEntity();

    virtual void Dispose();

    bool GetSyntheticBooleanMemberOrDefault(std::string memberName, bool defaultValue);

    int32_t GetSyntheticInt32MemberOrDefault(std::string memberName, int32_t defaultValue);

    float GetSyntheticSingleMemberOrDefault(std::string memberName, float defaultValue);

    std::string GetSyntheticStringMemberOrDefault(std::string memberName, std::string defaultValue);

    virtual void ParentEnabledChange(bool newEnabled);

    virtual void ParentStaticChange(bool newEnabled);

    void SetSyntheticBooleanMember(std::string memberName, bool value);

    void SetSyntheticInt32Member(std::string memberName, int32_t value);

    void SetSyntheticSingleMember(std::string memberName, float value);

    void SetSyntheticStringMember(std::string memberName, std::string value);

    void ThrowIfDisposed();
private:
    bool isDisposed;

    Dictionary<std::string, std::string>* SyntheticStringMembers;

    Dictionary<std::string, bool>* SyntheticBooleanMembers;

    Dictionary<std::string, int32_t>* SyntheticInt32Members;

    Dictionary<std::string, float>* SyntheticSingleMembers;
};
